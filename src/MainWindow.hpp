#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QStandardItemModel>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QProcess>
#include <QWidget>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableView>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QHeaderView>
#include <QList>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <QFile>
#include <QStringList>
#include <QOverload>
#include <QMetaObject>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

#include <memory>
#include <string_view>
#include <utility>

constexpr std::pair<int, int> DEFAULT_WINDOW_SIZE {1024, 768};
constexpr int PROCESS_FINISH_WAIT_MS = 5000;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_start_clicked();
    void on_stop_clicked();
    void on_process_output();
    void on_process_finished(int exit_code, QProcess::ExitStatus exit_status);
    // void on_hash_type_changed(int index);
    void on_engine_changed(int index);

private:
    enum class Engine { Hashcat, John };

    struct HashType {
        int hashcat_id;
        QString name;
        QString john_name; // John The Ripper format name
        QString example;
    };

    void setup_ui();
    void setup_connections();
    void load_settings();
    void save_settings();
    void parse_hashcat_output(std::string_view output);
    void parse_john_output(std::string_view output);
    void update_status(const QString& message);
    void update_hash_types();
    std::string_view trim_view(std::string_view sv);

    QProcess m_process;
    Engine m_current_engine = Engine::Hashcat;
    std::unique_ptr<QStandardItemModel> m_result_model;
    KConfigGroup m_config;
    std::vector<HashType> m_hash_types;
    std::unordered_map<int, int> m_hash_type_index_map; // Maps hash type IDs to combo box indices
    QFutureWatcher<void> m_output_parser_watcher;

    // UI Elements
    QComboBox* m_engine_combo { nullptr };
    QComboBox* m_hash_type_combo { nullptr };
    QComboBox* m_attack_mode_combo { nullptr };
    QLineEdit* m_hash_file_edit { nullptr };
    QLineEdit* m_word_list_edit { nullptr };
    QLineEdit* m_rules_edit { nullptr };
    QPlainTextEdit* m_output_edit { nullptr };
    QTableView* m_result_view { nullptr };
    QPushButton* m_start_button { nullptr };
    QPushButton* m_stop_button { nullptr };
    QProgressBar* m_progress_bar { nullptr };
    QLabel* m_status_label { nullptr };
};

#endif