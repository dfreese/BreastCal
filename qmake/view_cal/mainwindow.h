#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <miil/SystemConfiguration.h>
#include <vector>

namespace Ui {
class MainWindow;
}

class TH1F;
class TH2F;
class TGraph;
class TQtWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updatePlots();

    void action_Open();

    void on_pushButton_select_config_clicked();

    void on_pushButton_select_ped_clicked();

    void on_pushButton_select_pp_clicked();

    void on_pushButton_select_loc_clicked();

    void on_pushButton_select_cal_clicked();

    void on_pushButton_select_apd_clicked();

    void on_pushButton_select_flood_clicked();

    void on_pushButton_select_graph_clicked();

    void on_pushButton_select_crystal_clicked();

    void on_pushButton_load_config_clicked();

    void on_pushButton_load_cal_clicked();

    void on_pushButton_load_ped_clicked();

    void on_pushButton_load_loc_clicked();

    void on_pushButton_load_pp_clicked();

    void on_pushButton_load_apd_clicked();

    void on_pushButton_load_flood_clicked();

    void on_pushButton_load_graph_clicked();

    void on_pushButton_load_crystal_clicked();

    void on_spinBox_panel_valueChanged(int arg1);

    void on_spinBox_cartridge_valueChanged(int arg1);

    void on_spinBox_fin_valueChanged(int arg1);

    void on_spinBox_module_valueChanged(int arg1);

    void on_spinBox_apd_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QString last_directory;

    QString filepath_calibration_config;
    QString filepath_config;
    QString filepath_ped;
    QString filepath_pp;
    QString filepath_loc;
    QString filepath_cal;
    QString filepath_apd;
    QString filepath_flood;
    QString filepath_graph;
    QString filepath_crystal;
    QString filename_calibration_config;
    QString filename_config;
    QString filename_ped;
    QString filename_pp;
    QString filename_loc;
    QString filename_cal;
    QString filename_apd;
    QString filename_flood;
    QString filename_graph;
    QString filename_crystal;

    bool config_loaded;
    SystemConfiguration config;

    void setButtonsEnabled(bool val);

    std::vector<std::vector<std::vector<std::vector<
            std::vector<TH1F*> > > > > hists_apd_spat;
    std::vector<std::vector<std::vector<std::vector<
            std::vector<TH1F*> > > > > hists_apd_comm;
    std::vector<std::vector<std::vector<std::vector<
            std::vector<TH2F*> > > > > floods;
    std::vector<std::vector<std::vector<std::vector<
            std::vector<TGraph*> > > > > graphs;
    std::vector<std::vector<std::vector<std::vector<std::vector<
            std::vector<TH1F*> > > > > > hists_crystal_spat;
    std::vector<std::vector<std::vector<std::vector<std::vector<
            std::vector<TH1F*> > > > > > hists_crystal_comm;


    TQtWidget *rootWidget00;
    TQtWidget *rootWidget01;
    TQtWidget *rootWidget10;
    TQtWidget *rootWidget11;

    int current_panel;
    int current_cartridge;
    int current_fin;
    int current_module;
    int current_apd;

};

#endif // MAINWINDOW_H
