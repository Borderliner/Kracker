#include "MainWindow.hpp"
#include <ranges>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_config(KSharedConfig::openConfig(), "MainWindow")
    , m_result_model(std::make_unique<QStandardItemModel>()) {

    // Try multiple methods to find the icon
    QIcon icon;
    
    // 1. First try theme icon (works after installation)
    if (QIcon::hasThemeIcon("kracker")) {
        icon = QIcon::fromTheme("kracker");
    }
    // 2. Try build directory (debug mode)
    else if (QFile::exists("hicolor/512x512/apps/kracker.png")) {
        icon = QIcon("hicolor/512x512/apps/kracker.png");
    }
    // 3. Try source directory (alternative debug mode)
    else if (QFile::exists("../icons/hicolor/512x512/apps/kracker.png")) {
        icon = QIcon("../icons/hicolor/512x512/apps/kracker.png");
    }
    // 4. Fallback to embedded resource
    else {
        icon = QIcon(":/icons/kracker.png");
    }
    
    setWindowIcon(icon);
    resize(DEFAULT_WINDOW_SIZE.first, DEFAULT_WINDOW_SIZE.second);

    // Initialize hash types (common ones for both backends)
    m_hash_types = {
        // Format: {hashcat_id, jtr_name, name, example}
        { 0, "raw-md5", i18n("MD5"), "8743b52063cd84097a65d1633f5c74f5" },
        { 100, "raw-sha1", i18n("SHA1"), "b89eaac7e61417341b710b727768294d0e6a277b" },
        { 1000, "nt", i18n("NTLM"), "B4B9B02E6F09A9BD760F388B67351E2B" },
        { 1400, "raw-sha256", i18n("SHA-256"), "127e6fbfe24a750e72930c220a8e138275656b8e5d8f48a98c3c92df2caba935" },
        { 500, "md5crypt", i18n("MD5 Crypt"), "$1$salt$Xxd1irWTvBpPm8Z0Em/9f/" },
        { 1800, "sha512crypt", i18n("SHA-512 Crypt"), "$6$salt$qFmFH.bQsz7X0hS4UygfX0YQPFX0fUVNbMojFqGtukigIJmH6Yy4XGpxVgquV1XqS5rH/coOWd8WzJ5RzUxlW0" },
        { 3200, "bcrypt", i18n("bcrypt"), "$2a$05$bvIG6Nmid91Mu9RcmmWZfO5HJIMCT8riNW0hEp8f6/FuA2/mHZFpe" },
        { 7500, "Kerberos 5 AS-REQ Pre-Auth", i18n("Kerberos 5 AS-REQ Pre-Auth"), "$krb5pa$23$user$realm$salt$5f3d986f5e5844f2b0c3989f75a3c0d2" },
        { 13100, "Kerberos 5 TGS-REP", i18n("Kerberos 5 TGS-REP"), "$krb5tgs$23$*user$realm$test/spn*$63386d22d359fe42230300e568d57879$..." },
        { 10000, "Django (PBKDF2-SHA256)", i18n("Django (PBKDF2-SHA256)"), "pbkdf2_sha256$10000$salt$hash" },
        { 1700, "SHA-512", i18n("SHA-512"), "9b71d224bd62f3785d96d46ad3ea3d73319bfbc2890caadae2dff72519673ca72323c3d99ba5c11d7c7acc6e14b8c5da0c4663475c2e5c3adef46f73bcdec043" },
        { 3000, "LM", i18n("LM Hash"), "299BD128C1101FD6" },
        { 5500, "NetNTLMv1", i18n("NetNTLMv1"), "u4-netntlm::kNS:338d08f8e26de93300000000000000000000000000000000:..." },
        { 5600, "NetNTLMv2", i18n("NetNTLMv2"), "admin::N46iSNekpT:08ca45b7d7ea58ee:..." },
        { 7500, "Kerberos 5 AS-REQ Pre-Auth", i18n("Kerberos 5 AS-REQ Pre-Auth"), "$krb5pa$23$user$realm$salt$5f3d986f5e5844f2b0c3989f75a3c0d2" },
        { 13200, "AxCrypt", i18n("AxCrypt"), "$axcrypt$*1*10000*735c917ecb34e8d7*..." },
        { 6211, "TrueCrypt PBKDF2-HMAC-RIPEMD160 + XTS 512 bit", i18n("TrueCrypt PBKDF2-HMAC-RIPEMD160 + XTS 512 bit"), "$tc$16120$5f4dcc3b5aa765d61d8327deb882cf99$..." },
        { 13400, "KeePass 1 (AES/Twofish)", i18n("KeePass 1 (AES/Twofish)"), "$keepass$*1*50000*0*..." },
        { 6800, "LastPass", i18n("LastPass"), "$lastpass$..." },
        { 11300, "Bitcoin/Litecoin wallet.dat", i18n("Bitcoin/Litecoin wallet.dat"), "$bitcoin$96$..." }
    };

    setup_ui();
    setup_connections();
    load_settings();
}

MainWindow::~MainWindow() {
    save_settings();
    if (m_process.state() == QProcess::Running) {
        m_process.terminate();
        m_process.waitForFinished(PROCESS_FINISH_WAIT_MS);
    }
}

void MainWindow::setup_ui() {
    auto central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    // Create UI elements (same as before)
    m_engine_combo = new QComboBox();
    m_engine_combo->addItem(i18n("Hashcat"), QVariant::fromValue(Engine::Hashcat));
    m_engine_combo->addItem(i18n("John The Ripper"), QVariant::fromValue(Engine::John));

    m_hardware_combo = new QComboBox();
    m_hardware_combo->addItem(i18n("Auto Detect"), QVariant::fromValue(HardwareAccel::Auto));
    m_hardware_combo->addItem(i18n("CPU Only"), QVariant::fromValue(HardwareAccel::CPU));
    m_hardware_combo->addItem(i18n("NVIDIA CUDA"), QVariant::fromValue(HardwareAccel::NVIDIA_CUDA));
    m_hardware_combo->addItem(i18n("AMD OpenCL"), QVariant::fromValue(HardwareAccel::AMD_OpenCL));

    m_device_combo = new QComboBox();
    m_device_combo->setEnabled(false);
    m_device_combo->setMinimumWidth(150);

    m_optimized_kernel_check = new QCheckBox(i18n("Optimized Kernel"));
    m_optimized_kernel_check->setChecked(true);

    m_workload_profile_spin = new QSpinBox();
    m_workload_profile_spin->setRange(1, 4);
    m_workload_profile_spin->setValue(3);
    m_workload_profile_spin->setToolTip(i18n("1=Low, 2=Default, 3=High, 4=Nightmare"));

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

    m_result_model->setHorizontalHeaderLabels({ i18n("Hash"), i18n("Password") });
    m_result_view->setModel(m_result_model.get());
    m_result_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Populate initial hash types and attack modes
    update_hash_types();
    
    const QList<std::pair<int, QString>> attack_modes = {
        { 0, i18n("Straight (Dictionary)") },
        { 1, i18n("Combination") },
        { 3, i18n("Brute-Force") },
        { 6, i18n("Hybrid Wordlist + Mask") },
        { 7, i18n("Hybrid Mask + Wordlist") }
    };
    for (const auto& am : attack_modes) {
        m_attack_mode_combo->addItem(am.second, am.first);
    }

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

    // Setup layouts
    auto main_layout = new QVBoxLayout(central_widget);

    // Backend Group
    auto backend_group = new QGroupBox(i18n("Backend"));
    auto backend_layout = new QHBoxLayout();
    
    // Add backend controls to the horizontal layout
    backend_layout->addWidget(new QLabel(i18n("Engine:")));
    backend_layout->addWidget(m_engine_combo);
    backend_layout->addWidget(new QLabel(i18n("Hardware:")));
    backend_layout->addWidget(m_hardware_combo);
    backend_layout->addWidget(new QLabel(i18n("Device:")));
    backend_layout->addWidget(m_device_combo);
    backend_layout->addWidget(new QLabel(i18n("Performance:")));
    backend_layout->addWidget(m_workload_profile_spin);
    backend_layout->addWidget(m_optimized_kernel_check);
    backend_layout->addStretch(); // Push everything to the left
    
    backend_group->setLayout(backend_layout);

    // Definitions Group
    auto definitions_group = new QGroupBox(i18n("Definitions"));
    auto definitions_layout = new QFormLayout();
    
    // First row: Hash Type and Attack Mode
    auto hash_attack_layout = new QHBoxLayout();
    hash_attack_layout->addWidget(new QLabel(i18n("Hash Type:")));
    hash_attack_layout->addWidget(m_hash_type_combo);
    hash_attack_layout->addWidget(new QLabel(i18n("Attack Mode:")));
    hash_attack_layout->addWidget(m_attack_mode_combo);
    hash_attack_layout->addStretch();
    definitions_layout->addRow(hash_attack_layout);

    // File inputs with browse buttons
    auto hash_file_layout = new QHBoxLayout();
    hash_file_layout->addWidget(new QLabel(i18n("Hash File: ")));
    hash_file_layout->addWidget(m_hash_file_edit);
    hash_file_layout->addWidget(make_browse_button(m_hash_file_edit, i18n("Select Hash File"), i18n("Text Files (*.txt);;All Files (*)")));
    definitions_layout->addRow(hash_file_layout);

    auto word_list_layout = new QHBoxLayout();
    word_list_layout->addWidget(new QLabel(i18n("Wordlist:  ")));
    word_list_layout->addWidget(m_word_list_edit);
    word_list_layout->addWidget(make_browse_button(m_word_list_edit, i18n("Select Wordlist"), i18n("Wordlists (*.txt *.dic);;All Files (*)")));
    definitions_layout->addRow(word_list_layout);

    auto rules_layout = new QHBoxLayout();
    rules_layout->addWidget(new QLabel(i18n("Rules File:")));
    rules_layout->addWidget(m_rules_edit);
    rules_layout->addWidget(make_browse_button(m_rules_edit, i18n("Select Rules File"), i18n("Rules (*.rule);;All Files (*)")));
    definitions_layout->addRow(rules_layout);

    definitions_group->setLayout(definitions_layout);

    // Button layout
    auto button_layout = new QHBoxLayout();
    button_layout->addWidget(m_start_button);
    button_layout->addWidget(m_stop_button);
    button_layout->addStretch();

    // Main layout organization
    main_layout->addWidget(backend_group);
    main_layout->addWidget(definitions_group);
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
    connect(m_engine_combo, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::on_engine_changed);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::on_process_output);
    connect(&m_process, &QProcess::readyReadStandardError, this, &MainWindow::on_process_output);
    connect(&m_process, &QProcess::finished, this, &MainWindow::on_process_finished);
    connect(m_hardware_combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_current_hardware = static_cast<HardwareAccel>(m_hardware_combo->itemData(index).toInt());
        m_device_combo->setEnabled(m_current_hardware != HardwareAccel::Auto && 
                                m_current_hardware != HardwareAccel::CPU);
        update_device_list();
    });
}

void MainWindow::update_hash_types() {
    m_hash_type_combo->clear();
    m_hash_type_index_map.clear();
    
    int index = 0;
    for (const auto& ht : m_hash_types) {
        if (m_current_engine == Engine::Hashcat) {
            m_hash_type_combo->addItem(ht.name, ht.hashcat_id);
            m_hash_type_index_map[ht.hashcat_id] = index++;
        } else { // John The Ripper
            m_hash_type_combo->addItem(ht.name, ht.john_name);
            // For John, just use the index directly since we're using strings
        }
    }
}

void MainWindow::update_device_list() {
    m_device_combo->clear();
    
    if (m_current_engine != Engine::Hashcat || 
        m_current_hardware == HardwareAccel::Auto || 
        m_current_hardware == HardwareAccel::CPU) {
        return;
    }

    // Run hashcat with --benchmark to detect devices
    QProcess detect_process;
    QStringList args = {"--benchmark", "--quiet"};
    
    if (m_current_hardware == HardwareAccel::NVIDIA_CUDA) {
        args << "--opencl-device-types" << "2"; // CUDA
    } else if (m_current_hardware == HardwareAccel::AMD_OpenCL) {
        args << "--opencl-device-types" << "1"; // GPU
    }

    detect_process.start("hashcat", args);
    detect_process.waitForFinished();
    
    QString output = detect_process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    // Parse device info (TODO: real parsing would need more complex regex)
    for (const QString& line : lines) {
        if (line.contains("Device #")) {
            QString device_name = line.section(':', 1).trimmed();
            int device_id = line.section('#', 1).section(':', 0, 0).toInt();
            m_device_combo->addItem(QString("Device %1: %2").arg(device_id).arg(device_name), device_id);
        }
    }
    
    if (m_device_combo->count() == 0) {
        m_device_combo->addItem(i18n("No compatible devices found"), -1);
    }
}

void MainWindow::load_settings() {
    m_config.config()->reparseConfiguration();
    restoreGeometry(m_config.readEntry("WindowGeometry", QByteArray()));
    m_engine_combo->setCurrentIndex(m_config.readEntry("LastEngine", 0));
    m_hardware_combo->setCurrentIndex(m_config.readEntry("LastHardware", 0));
    m_hash_file_edit->setText(m_config.readEntry("LastHashFile"));
    m_word_list_edit->setText(m_config.readEntry("LastWordlist"));
    m_rules_edit->setText(m_config.readEntry("LastRulesFile"));
    m_hash_type_combo->setCurrentIndex(m_config.readEntry("LastHashType", 0));
    m_attack_mode_combo->setCurrentIndex(m_config.readEntry("LastAttackMode", 0));
    m_optimized_kernel_check->setChecked(m_config.readEntry("OptimizedKernel", true));
    m_workload_profile_spin->setValue(m_config.readEntry("WorkloadProfile", 3));
}

void MainWindow::save_settings() {
    m_config.writeEntry("WindowGeometry", saveGeometry());
    m_config.writeEntry("LastEngine", m_engine_combo->currentIndex());
    m_config.writeEntry("LastHardware", m_hardware_combo->currentIndex());
    m_config.writeEntry("LastHashFile", m_hash_file_edit->text());
    m_config.writeEntry("LastWordlist", m_word_list_edit->text());
    m_config.writeEntry("LastRulesFile", m_rules_edit->text());
    m_config.writeEntry("LastHashType", m_hash_type_combo->currentIndex());
    m_config.writeEntry("LastAttackMode", m_attack_mode_combo->currentIndex());
    m_config.writeEntry("OptimizedKernel", m_optimized_kernel_check->isChecked());
    m_config.writeEntry("WorkloadProfile", m_workload_profile_spin->value());
    m_config.sync();
}

void MainWindow::on_engine_changed(int index) {
    m_current_engine = static_cast<Engine>(m_engine_combo->itemData(index).toInt());
    update_hash_types();

    // Enable/disable hardware acceleration controls based on engine
    bool is_hashcat = (m_current_engine == Engine::Hashcat);
    m_hardware_combo->setEnabled(is_hashcat);
    m_device_combo->setEnabled(is_hashcat && m_current_hardware != HardwareAccel::Auto && 
                            m_current_hardware != HardwareAccel::CPU);
    m_optimized_kernel_check->setCheckState(is_hashcat ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    m_optimized_kernel_check->setEnabled(is_hashcat);
    m_workload_profile_spin->setEnabled(is_hashcat);

    // Update attack mode combo
    m_attack_mode_combo->clear();
    if (m_current_engine == Engine::Hashcat) {
        const QList<std::pair<int, QString>> attack_modes = {
            { 0, i18n("Straight (Dictionary)") },
            { 1, i18n("Combination") },
            { 3, i18n("Brute-Force") },
            { 6, i18n("Hybrid Wordlist + Mask") },
            { 7, i18n("Hybrid Mask + Wordlist") }
        };
        for (const auto& am : attack_modes) {
            m_attack_mode_combo->addItem(am.second, am.first);
        }
    } else { // John The Ripper
        const QList<std::pair<QString, QString>> attack_modes = {
            { "wordlist", i18n("Dictionary") },
            { "single", i18n("Single Crack") },
            { "incremental", i18n("Incremental") },
            { "external", i18n("External") }
        };
        for (const auto& am : attack_modes) {
            m_attack_mode_combo->addItem(am.second, am.first);
        }
    }
}

void MainWindow::on_start_clicked() {
    if (m_process.state() == QProcess::Running) {
        QMessageBox::information(this, i18n("Process Running"), i18n("Cracker is already running."));
        return;
    }

    const auto hash_file = m_hash_file_edit->text();
    if (hash_file.isEmpty() || !QFile::exists(hash_file)) {
        QMessageBox::warning(this, i18n("Input Error"), i18n("Please specify a valid hash file."));
        return;
    }

    QStringList args;
    QString program;

    if (m_current_engine == Engine::Hashcat) {
        program = "hashcat";
        args = {
            "--hash-type", QString::number(m_hash_type_combo->currentData().toInt()),
            "--attack-mode", QString::number(m_attack_mode_combo->currentData().toInt()),
            "--potfile-disable", // Don't use potfile to see all results
            hash_file
        };

        // Add hardware acceleration options
        if (m_current_hardware != HardwareAccel::Auto) {
            if (m_current_hardware == HardwareAccel::CPU) {
                args << "--opencl-device-types" << "1"; // CPU only
            } else if (m_current_hardware == HardwareAccel::NVIDIA_CUDA) {
                args << "--opencl-device-types" << "2"; // NVIDIA CUDA
            } else if (m_current_hardware == HardwareAccel::AMD_OpenCL) {
                args << "--opencl-device-types" << "1"; // AMD GPU (uses OpenCL)
            }
            
            // Add specific device if selected
            if (m_device_combo->isEnabled() && m_device_combo->currentData().toInt() >= 0) {
                args << "--opencl-devices" << m_device_combo->currentData().toString();
            }
        }

        // Add performance options
        args << "--workload-profile" << QString::number(m_workload_profile_spin->value());

        if (m_optimized_kernel_check->isChecked()) {
            args << "-O"; // Optimized kernel
        }

        // Add force flag for GPU modes to ignore warnings
        if (m_current_hardware != HardwareAccel::CPU) {
            args << "--force";
        }

        // Add wordlist if specified
        const auto wordlist = m_word_list_edit->text();
        if (!wordlist.isEmpty()) {
            if (!QFile::exists(wordlist)) {
                QMessageBox::warning(this, i18n("Input Error"), i18n("Wordlist file doesn't exist."));
                return;
            }
            args << wordlist;
        }

        // Add rules if specified
        const auto rules = m_rules_edit->text();
        if (!rules.isEmpty()) {
            if (!QFile::exists(rules)) {
                QMessageBox::warning(this, i18n("Input Error"), i18n("Rules file doesn't exist."));
                return;
            }
            args << "--rules-file" << rules;
        }

    } else { // John The Ripper
        program = "john";
        args = {
            "--format=" + m_hash_type_combo->currentData().toString(),
            hash_file
        };

        // Set attack mode
        QString attack_mode = m_attack_mode_combo->currentData().toString();
        if (attack_mode == "wordlist") {
            const auto wordlist = m_word_list_edit->text();
            if (!wordlist.isEmpty()) {
                if (!QFile::exists(wordlist)) {
                    QMessageBox::warning(this, i18n("Input Error"), i18n("Wordlist file doesn't exist."));
                    return;
                }
                args << "--wordlist=" + wordlist;
            }
        } else {
            args << "--" + attack_mode;
        }

        // Add rules if specified
        const auto rules = m_rules_edit->text();
        if (!rules.isEmpty()) {
            if (!QFile::exists(rules)) {
                QMessageBox::warning(this, i18n("Input Error"), i18n("Rules file doesn't exist."));
                return;
            }
            args << "--rules=" + QFileInfo(rules).baseName();
        }

        // Show cracked passwords
        args << "--show";
    }

    m_output_edit->clear();
    m_result_model->clear();
    m_result_model->setHorizontalHeaderLabels({ i18n("Hash"), i18n("Password") });

    update_status(i18n("Starting %1...").arg(program));
    m_output_edit->appendPlainText(program + " " + args.join(' '));

    m_process.start(program, args);
    m_start_button->setEnabled(false);
    m_stop_button->setEnabled(true);
    m_progress_bar->setRange(0, 0); // Indeterminate progress
}

void MainWindow::on_stop_clicked() {
    if (m_process.state() == QProcess::Running) {
        m_process.terminate();
        update_status(i18n("Stopping process..."));
    }
}

void MainWindow::on_process_output() {
    const auto output = m_process.readAllStandardOutput().toStdString();
    const auto error = m_process.readAllStandardError().toStdString();

    // Use QtConcurrent to parse output in background
    m_output_parser_watcher.setFuture(QtConcurrent::run([this, output, error] {
        if (m_current_engine == Engine::Hashcat) {
            parse_hashcat_output(output);
            if (!error.empty()) {
                parse_hashcat_output(error);
            }
        } else {
            parse_john_output(output);
            if (!error.empty()) {
                parse_john_output(error);
            }
        }
    }));
}

void MainWindow::parse_hashcat_output(std::string_view output) {
    QString result;
    QList<QList<QStandardItem*>> new_rows;

    for (auto line : std::ranges::views::split(output, '\n')) {
        std::string_view line_sv(line.begin(), line.end());

        if (line_sv.empty()) continue;

        // Status updates
        if (line_sv.starts_with("Status:")) {
            const auto msg = QString::fromUtf8(line_sv.data(), line_sv.size());
            QMetaObject::invokeMethod(this, [this, msg] {
                update_status(msg);
            });
            continue;
        }

        // Progress update
        if (line_sv.find("Progress:") != std::string_view::npos) {
            size_t pct_start = line_sv.find(":") + 1;
            size_t pct_end = line_sv.find("%");
            if (pct_start < pct_end) {
                auto pct_str = line_sv.substr(pct_start, pct_end - pct_start);
                int pct = 0;
                std::from_chars(pct_str.data(), pct_str.data() + pct_str.size(), pct);

                QMetaObject::invokeMethod(this, [this, pct] {
                    m_progress_bar->setRange(0, 100);
                    m_progress_bar->setValue(pct);
                });
            }
            continue;
        }

        // Found hashes
        if (line_sv.find(':') != std::string_view::npos) {
            size_t colon_pos = line_sv.find(':');
            if (colon_pos != std::string_view::npos) {
                std::string_view hash = line_sv.substr(0, colon_pos);
                std::string_view password = line_sv.substr(colon_pos + 1);
                
                hash = trim_view(hash);
                password = trim_view(password);

                auto* hash_item = new QStandardItem(QString::fromUtf8(hash.data(), hash.size()));
                auto* pass_item = new QStandardItem(QString::fromUtf8(password.data(), password.size()));

                QMetaObject::invokeMethod(this, [this, hash_item, pass_item] {
                    m_result_model->appendRow({hash_item, pass_item});
                });
            }
        }
        result.append(QString::fromUtf8(line_sv.data(), line_sv.size())).append('\n');
    }

    if (!result.isEmpty()) {
        QMetaObject::invokeMethod(m_output_edit, [this, result] {
            m_output_edit->appendPlainText(result);
        });
    }
}

void MainWindow::parse_john_output(std::string_view output) {
    QString result;
    QList<QList<QStandardItem*>> new_rows;

    for (auto line : std::ranges::views::split(output, '\n')) {
        std::string_view line_sv(line.begin(), line.end());

        if (line_sv.empty()) continue;

        // Status updates
        if (line_sv.find("Loaded") != std::string_view::npos || 
            line_sv.find("guesses:") != std::string_view::npos) {
            const auto msg = QString::fromUtf8(line_sv.data(), line_sv.size());
            QMetaObject::invokeMethod(this, [this, msg] {
                update_status(msg);
            });
        }

        // Progress update (JtR doesn't provide precise progress)
        if (line_sv.find("Current") != std::string_view::npos) {
            QMetaObject::invokeMethod(this, [this] {
                if (m_progress_bar->maximum() == 0) {
                    m_progress_bar->setRange(0, 100);
                }
                // Just pulse the progress bar since JtR doesn't give precise progress
                m_progress_bar->setValue((m_progress_bar->value() + 5) % 100);
            });
        }

        // Found hashes (JtR format: username:password)
        if (line_sv.find(':') != std::string_view::npos && 
            line_sv.find("password hash") == std::string_view::npos) {
            size_t colon_pos = line_sv.find(':');
            if (colon_pos != std::string_view::npos) {
                std::string_view hash_part = line_sv.substr(0, colon_pos);
                std::string_view password_part = line_sv.substr(colon_pos + 1);
                
                // Extract hash from the first part (username:hash)
                size_t hash_colon = hash_part.rfind(':');
                std::string_view hash = (hash_colon != std::string_view::npos) ? 
                    hash_part.substr(hash_colon + 1) : hash_part;
                
                hash = trim_view(hash);
                password_part = trim_view(password_part);

                auto* hash_item = new QStandardItem(QString::fromUtf8(hash.data(), hash.size()));
                auto* pass_item = new QStandardItem(QString::fromUtf8(password_part.data(), password_part.size()));

                QMetaObject::invokeMethod(this, [this, hash_item, pass_item] {
                    m_result_model->appendRow({hash_item, pass_item});
                });
            }
        }
        result.append(QString::fromUtf8(line_sv.data(), line_sv.size())).append('\n');
    }

    if (!result.isEmpty()) {
        QMetaObject::invokeMethod(m_output_edit, [this, result] {
            m_output_edit->appendPlainText(result);
        });
    }
}

std::string_view MainWindow::trim_view(std::string_view sv) {
    constexpr auto whitespace = " \t\n\r\f\v";
    sv.remove_prefix(std::min(sv.find_first_not_of(whitespace), sv.size()));
    sv.remove_suffix(std::min(sv.size() - sv.find_last_not_of(whitespace) - 1, sv.size()));
    return sv;
}

void MainWindow::on_process_finished(int exit_code, QProcess::ExitStatus exit_status) {
    Q_UNUSED(exit_status)

    m_start_button->setEnabled(true);
    m_stop_button->setEnabled(false);
    m_progress_bar->setRange(0, 100);

    if (exit_code == 0) {
        update_status(i18n("Process finished successfully"));
    } else {
        update_status(i18n("Process finished with error code: %1").arg(exit_code));
    }
}

void MainWindow::update_status(const QString& message) {
    m_status_label->setText(message);
}
