#include "MainWindow.hpp"
#include <qtpreprocessorsupport.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_config(KSharedConfig::openConfig(), "MainWindow")
    , m_result_model(std::make_unique<QStandardItemModel>()) {

    setWindowTitle(i18n("KHashcat"));
    resize(DEFAULT_WINDOW_SIZE.first, DEFAULT_WINDOW_SIZE.second);

    // Initialize hash types
    m_hash_types = {
        { 0, i18n("MD5"), "8743b52063cd84097a65d1633f5c74f5" },
        { 100, i18n("SHA1"), "b89eaac7e61417341b710b727768294d0e6a277b" },
        { 1000, i18n("NTLM"), "B4B9B02E6F09A9BD760F388B67351E2B" },
        { 1400, i18n("SHA-256"), "127e6fbfe24a750e72930c220a8e138275656b8e5d8f48a98c3c92df2caba935" }
    };

    setup_ui();
    setup_connections();
    load_settings();
}

MainWindow::~MainWindow() {
    save_settings();
    if (m_hashcat_process.state() == QProcess::Running) {
        m_hashcat_process.terminate();
        m_hashcat_process.waitForFinished(5000);
    }
}

void MainWindow::setup_ui() {
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    // Create UI elements
    m_hash_type_combo = new QComboBox();
    m_attack_mode_combo = new QComboBox();
    m_hash_file_edit = new QLineEdit();
    m_word_list_edit = new QLineEdit();
    m_rules_edit = new QLineEdit();
    m_output_edit = new QPlainTextEdit();
    m_output_edit->setReadOnly(true);
    m_start_button = new QPushButton(i18n("Start"));
    m_stop_button = new QPushButton(i18n("Stop"));
    m_stop_button->setEnabled(false);
    m_result_view = new QTableView();
    m_progress_bar = new QProgressBar();
    m_status_label = new QLabel();

    m_result_model->setHorizontalHeaderLabels({ i18n("Hash"), i18n("Password")  });
    m_result_view->setModel(m_result_model.get());
    m_result_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Populate hash types using C++20 ranges
    std::ranges::for_each(m_hash_types, [this](const HashType& ht) {
        m_hash_type_combo->addItem(ht.name, ht.id);
    });

    // Attack modes
    const QList<std::pair<int, QString>> attack_modes = {
        { 0, i18n("Straight") },
        { 1, i18n("Combination") },
        { 3, i18n("Brute-Force") },
        { 6, i18n("Hybrid Wordlist + Mask") },
        { 7, i18n("Hybrid Mask + Wordlist") }
    };

    std::ranges::for_each(attack_modes, [this](const auto& am) {
        m_attack_mode_combo->addItem(am.second, am.first);
    });

    // Setup layouts
    auto main_layout = new QVBoxLayout(central_widget);

    // Input group
    auto input_group = new QGroupBox(i18n("Input Parameters"));
    auto form_layout = new QFormLayout();

    // Helper lambda for creating browse buttons
    auto make_browse_button = [this](QLineEdit* line_edit, const QString& title, const QString& filter) {
        auto button = new QPushButton("...");
        button->setFixedWidth(30);
        connect(button, &QPushButton::clicked, this, [this, line_edit, title, filter]() {
            auto file_name = QFileDialog::getOpenFileName(this, title, {}, filter);            
            if (!file_name.isEmpty()) {
                line_edit->setText(file_name);
            }
        });
        return button;
    };

    auto hash_file_layout = new QHBoxLayout();
    hash_file_layout->addWidget(m_hash_file_edit);
    hash_file_layout->addWidget(make_browse_button(m_hash_file_edit, i18n("Select Hash File"), i18n("Text Files (*.txt);;All Files (*)")));

    auto word_list_layout = new QHBoxLayout();
    word_list_layout->addWidget(m_word_list_edit);
    word_list_layout->addWidget(make_browse_button(m_word_list_edit, i18n("Select Wordlist"), i18n("Wordlists (*.txt *.dic);;All Files (*)")));

    auto rules_layout = new QHBoxLayout();
    rules_layout->addWidget(m_rules_edit);
    rules_layout->addWidget(make_browse_button(m_rules_edit, i18n("Select Rules File"), i18n("Rules (*.rule);;All Files (*)")));

    form_layout->addRow(i18n("Hash Type:"), m_hash_type_combo);
    form_layout->addRow(i18n("Attack Mode:"), m_attack_mode_combo);
    form_layout->addRow(i18n("Hash File:"), hash_file_layout);
    form_layout->addRow(i18n("Wordlist:"), word_list_layout);
    form_layout->addRow(i18n("Rules File:"), rules_layout);
    input_group->setLayout(form_layout);

    // Button layout
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(m_start_button);
    button_layout->addWidget(m_stop_button);
    button_layout->addStretch();

    main_layout->addWidget(input_group);
    main_layout->addWidget(new QLabel(i18n("Output:")));
    main_layout->addWidget(m_output_edit);
    main_layout->addWidget(m_progress_bar);
    main_layout->addWidget(new QLabel(i18n("Results:")));
    main_layout->addWidget(m_result_view);
    main_layout->addLayout(button_layout);
    main_layout->addWidget(m_status_label);

    update_status(i18n("Ready"));
}

void MainWindow::setup_connections() {
    connect(m_start_button, &QPushButton::clicked, this, &MainWindow::on_start_clicked);
    connect(m_stop_button, &QPushButton::clicked, this, &MainWindow::on_stop_clicked);
    //connect(m_hash_type_combo, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::on_hash_type_changed);
    connect(&m_hashcat_process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_process_output);
    connect(&m_hashcat_process, &QProcess::readyReadStandardError, this, &MainWindow::on_process_output);
    connect(&m_hashcat_process, &QProcess::finished, this, &MainWindow::on_process_finished);
}

void MainWindow::load_settings() {
    m_config.config()->reparseConfiguration();
    restoreGeometry(m_config.readEntry("WindowGeometry", QByteArray()));
    m_hash_file_edit->setText(m_config.readEntry("LastHashFile"));
    m_word_list_edit->setText(m_config.readEntry("LastWordlist"));
    m_rules_edit->setText(m_config.readEntry("LastRulesFile"));
    m_hash_type_combo->setCurrentIndex(m_config.readEntry("LastHashType", 0));
    m_attack_mode_combo->setCurrentIndex(m_config.readEntry("LastAttackMode", 0));
}

void MainWindow::save_settings() {
    m_config.writeEntry("WindowGeometry", saveGeometry());
    m_config.writeEntry("LastHashFile", m_hash_file_edit->text());
    m_config.writeEntry("LastWordlist", m_word_list_edit->text());
    m_config.writeEntry("LastRulesFile", m_rules_edit->text());
    m_config.writeEntry("LastHashType", m_hash_type_combo->currentIndex());
    m_config.writeEntry("LastAttackMode", m_attack_mode_combo->currentIndex());
    m_config.sync();
}

void MainWindow::on_start_clicked() {
    if (m_hashcat_process.state() == QProcess::Running) {
        QMessageBox::information(this, i18n("Process Running"), i18n("KHashCat is already running."));
        return;
    }

    const auto hash_file = m_hash_file_edit->text();
    if (hash_file.isEmpty() || !QFile::exists(hash_file)) {
        QMessageBox::warning(this, i18n("Input Error"), i18n("Please specify a valid hash file."));
        return;
    }

    QStringList args = {
        "--hash-type", QString::number(m_hash_type_combo->currentData().toInt()),
        "--attack-mode", QString::number(m_attack_mode_combo->currentData().toInt()),
        "--potfile-disable", // Don't use potfile to see all results
        "--force", // Ignore warnings
        hash_file
    };

    const auto wordlist = m_word_list_edit->text();
    if (!wordlist.isEmpty()) {
        if (!QFile::exists(wordlist)) {
            QMessageBox::warning(this, i18n("Input Error"), i18n("Wordlist file doesn't exist."));
            return;
        }
        args << wordlist;
    }

    const auto rules = m_rules_edit->text();
    if (!rules.isEmpty()) {
        if (!QFile::exists(rules)) {
            QMessageBox::warning(this, i18n("Input Error"), i18n("Rules file doesn't exist."));
            return;
        }
        args << "--rules-file" << rules;
    }

    m_output_edit->clear();
    m_result_model->clear();
    m_result_model->setHorizontalHeaderLabels({ i18n("Hash"), i18n("Password") });

    update_status(i18n("Starting hashcat..."));
    m_output_edit->appendPlainText("hashcat " + args.join(' '));

    m_hashcat_process.start("hashcat", args);
    m_start_button->setEnabled(false);
    m_stop_button->setEnabled(true);
    m_progress_bar->setRange(0, 0); // Indeterminate progress
}

void MainWindow::on_stop_clicked() {
    if (m_hashcat_process.state() == QProcess::Running) {
        m_hashcat_process.terminate();
        update_status(i18n("Stopping hashcat..."));
    }
}

void MainWindow::on_process_output() {
    const auto output = m_hashcat_process.readAllStandardOutput().toStdString();
    const auto error = m_hashcat_process.readAllStandardError().toStdString();

    // Use QtConcurrent to parse output in background
    m_output_parser_watcher.setFuture(QtConcurrent::run([this, output, error] {
        parse_hashcat_output(output);
        if (!error.empty()) {
            parse_hashcat_output(error);
        }
    }));
}

void MainWindow::parse_hashcat_output(std::string_view output) {
    // This runs in a background thread
    QString result;
    QList<QList<QStandardItem*>> new_rows;

    // Simple parsing - TODO: Make it more sophisticated
    /* for (auto line : std::ranges::views::split(output, '\n')) {

    } */
}

void MainWindow::on_process_finished(int exit_code, QProcess::ExitStatus exit_status) {
    Q_UNUSED(exit_status)

    m_start_button->setEnabled(true);
    m_stop_button->setEnabled(false);
    m_progress_bar->setRange(0, 100);

    if (exit_code == 0) {
        update_status(i18n("Hashcat finished successfully"));
    } else {
        update_status(i18n("Hashcat finished with error code: %1").arg(exit_code));
    }
}

void MainWindow::update_status(const QString& message) {
    m_status_label->setText(message);
}
