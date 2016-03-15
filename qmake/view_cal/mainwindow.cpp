#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    setButtonsEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setButtonsEnabled(bool val)
{
    ui->lineEdit_ped->setEnabled(val);
    ui->lineEdit_pp->setEnabled(val);
    ui->lineEdit_loc->setEnabled(val);
    ui->lineEdit_cal->setEnabled(val);
    ui->lineEdit_apd->setEnabled(val);
    ui->lineEdit_flood->setEnabled(val);
    ui->lineEdit_graph->setEnabled(val);
    ui->lineEdit_crystal->setEnabled(val);

    ui->pushButton_select_ped->setEnabled(val);
    ui->pushButton_select_pp->setEnabled(val);
    ui->pushButton_select_loc->setEnabled(val);
    ui->pushButton_select_cal->setEnabled(val);
    ui->pushButton_select_apd->setEnabled(val);
    ui->pushButton_select_flood->setEnabled(val);
    ui->pushButton_select_graph->setEnabled(val);
    ui->pushButton_select_crystal->setEnabled(val);

    ui->pushButton_load_ped->setEnabled(val);
    ui->pushButton_load_pp->setEnabled(val);
    ui->pushButton_load_loc->setEnabled(val);
    ui->pushButton_load_apd->setEnabled(val);
    ui->pushButton_load_flood->setEnabled(val);
    ui->pushButton_load_graph->setEnabled(val);
    ui->pushButton_load_crystal->setEnabled(val);
}

void MainWindow::on_pushButton_select_config_clicked()
{
    QString full_filename_config = QFileDialog::getOpenFileName(
                this,
                "Select System Config File",
                last_directory,
                "Json File (*.json);; All Files (*)");


    QFileInfo config_file_info(full_filename_config);
    filename_config = config_file_info.fileName();
    filepath_config = config_file_info.filePath();

    ui->lineEdit_config->setText(full_filename_config);
    last_directory = filepath_config;
}

void MainWindow::on_pushButton_select_ped_clicked()
{
    QString full_filename_ped = QFileDialog::getOpenFileName(
                this,
                "Select Pedestal File",
                last_directory,
                "Pedestal Files (*.ped);; All Files (*)");


    QFileInfo ped_file_info(full_filename_ped);
    filename_ped = ped_file_info.fileName();
    filepath_ped = ped_file_info.filePath();

    ui->lineEdit_ped->setText(full_filename_ped);
    last_directory = filepath_ped;
}

void MainWindow::on_pushButton_select_pp_clicked()
{
    QString full_filename_pp = QFileDialog::getOpenFileName(
                this,
                "Select Photopeak File",
                last_directory,
                "Photopeak Files (*.pp);; All Files (*)");


    QFileInfo pp_file_info(full_filename_pp);
    filename_pp = pp_file_info.fileName();
    filepath_pp = pp_file_info.filePath();

    ui->lineEdit_pp->setText(full_filename_pp);
    last_directory = filepath_pp;
}

void MainWindow::on_pushButton_select_loc_clicked()
{
    QString full_filename_loc = QFileDialog::getOpenFileName(
                this,
                "Select Crystal Location File",
                last_directory,
                "Location Files (*.loc);; All Files (*)");


    QFileInfo loc_file_info(full_filename_loc);
    filename_loc = loc_file_info.fileName();
    filepath_loc = loc_file_info.filePath();

    ui->lineEdit_loc->setText(full_filename_loc);
    last_directory = filepath_loc;
}

void MainWindow::on_pushButton_select_cal_clicked()
{
    QString full_filename_cal = QFileDialog::getOpenFileName(
                this,
                "Select Calibration File",
                last_directory,
                "Cal File (*.cal);; All Files (*)");


    QFileInfo cal_file_info(full_filename_cal);
    filename_cal = cal_file_info.fileName();
    filepath_cal = cal_file_info.filePath();

    ui->lineEdit_cal->setText(full_filename_cal);
    last_directory = filepath_cal;
}

void MainWindow::on_pushButton_select_apd_clicked()
{
    QString full_filename_apd = QFileDialog::getOpenFileName(
                this,
                "Select apd hist file",
                last_directory,
                "APD Photopeak Files (*.pp.root);;Root Files (*.root);; All Files (*)");


    QFileInfo apd_file_info(full_filename_apd);
    filename_apd = apd_file_info.fileName();
    filepath_apd = apd_file_info.filePath();

    ui->lineEdit_apd->setText(full_filename_apd);
    last_directory = filepath_apd;
}

void MainWindow::on_pushButton_select_flood_clicked()
{
    QString full_filename_flood = QFileDialog::getOpenFileName(
                this,
                "Select flood hist file",
                last_directory,
                "Flood Files (*.floods.root);;Root Files (*.root);; All Files (*)");


    QFileInfo flood_file_info(full_filename_flood);
    filename_flood = flood_file_info.fileName();
    filepath_flood = flood_file_info.filePath();

    ui->lineEdit_flood->setText(full_filename_flood);
    last_directory = filepath_flood;
}


void MainWindow::on_pushButton_select_graph_clicked()
{
    QString full_filename_graph = QFileDialog::getOpenFileName(
                this,
                "Select Graph File",
                last_directory,
                "Graphs Files (*.graphs.root);;Root Files (*.root);; All Files (*)");


    QFileInfo graph_file_info(full_filename_graph);
    filename_graph = graph_file_info.fileName();
    filepath_graph = graph_file_info.filePath();

    ui->lineEdit_graph->setText(full_filename_graph);
    last_directory = filepath_graph;
}

void MainWindow::on_pushButton_select_crystal_clicked()
{
    QString full_filename_crystal = QFileDialog::getOpenFileName(
                this,
                "Select Crystal Histogram File",
                last_directory,
                "Crystal Hist Files (*.xpp.root);;Root Files (*.root);; All Files (*)");


    QFileInfo ped_file_info(full_filename_crystal);
    filename_ped = ped_file_info.fileName();
    filepath_ped = ped_file_info.filePath();

    ui->lineEdit_ped->setText(full_filename_crystal);
    last_directory = filepath_ped;
}

void MainWindow::on_pushButton_load_config_clicked()
{

    int load_status = config.load(ui->lineEdit_config->text().toStdString());

    if (load_status < 0) {
        QMessageBox::warning(this, "Config Load Failed",
                             "Failed to load config file: " + filename_config);
        setButtonsEnabled(false);
        return;
    }

    // Resize all of the histogram and graph arrays to the current config.  The
    // other calibrations are stored inside of the config call itself.
    config.resizeArrayPCFMA<TH1F*>(hists_apd_spat, 0);
    config.resizeArrayPCFMA<TH1F*>(hists_apd_comm, 0);
    config.resizeArrayPCFMA<TH2F*>(floods, 0);
    config.resizeArrayPCFMA<TGraph*>(graphs, 0);
    config.resizeArrayPCFMAX<TH1F*>(hists_crystal_spat, 0);
    config.resizeArrayPCFMAX<TH1F*>(hists_crystal_comm, 0);
    // Now that the arrays have been resized, and the config loaded, let the
    // other buttons be clicked
    setButtonsEnabled(true);
}

void MainWindow::on_pushButton_load_ped_clicked()
{
    int load_status = config.loadPedestals(
                ui->lineEdit_ped->text().toStdString());

    if (load_status < 0) {
        QMessageBox::warning(this, "Pedestal Load Failed",
                             "Failed to load ped file: " + filename_ped);
        return;
    }
}

void MainWindow::on_pushButton_load_loc_clicked()
{
    int load_status = config.loadCrystalLocations(
                ui->lineEdit_loc->text().toStdString());

    if (load_status < 0) {
        QMessageBox::warning(this, "Crystal Location Load Failed",
                             "Failed to load loc file: " + filename_loc);
        return;
    }
}

void MainWindow::on_pushButton_load_pp_clicked()
{
    int load_status = config.loadPhotopeakPositions(
                ui->lineEdit_pp->text().toStdString());

    if (load_status < 0) {
        QMessageBox::warning(this, "Photopeak Position Load Failed",
                             "Failed to load pp file: " + filename_pp);
        return;
    }
}

void MainWindow::on_pushButton_load_cal_clicked()
{
    int load_status = config.loadCalibration(
                ui->lineEdit_cal->text().toStdString());

    if (load_status < 0) {
        QMessageBox::warning(this, "Cal Load Failed",
                             "Failed to load cal file: " + filename_cal);
        return;
    }
}

void MainWindow::on_pushButton_load_apd_clicked()
{
    TFile input_file(ui->lineEdit_apd->text().toAscii());
    if (input_file.IsZombie()) {
        QMessageBox::warning(this, "APD Load Failed",
                             "Failed to open apd file: " + filename_apd);
        return;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        stringstream name_spat;
                        name_spat << "E[" << p << "]["
                                  << c << "][" << f << "]["
                                  << m << "][" << a << "]";
                        stringstream name_comm;
                        name_comm << "E_com[" << p << "]["
                                  << c << "][" << f << "]["
                                  << m << "][" << a << "]";

                        TH1F * hist_spat = (TH1F*) input_file.Get(
                                    name_spat.str().c_str());
                        if (hist_spat) {
                            hists_apd_spat[p][c][f][m][a] =
                                    (TH1F*) hist_spat->Clone(
                                        name_spat.str().c_str());
                        }
                        TH1F * hist_comm = (TH1F*) input_file.Get(
                                    name_comm.str().c_str());
                        if (hist_spat) {
                            hists_apd_comm[p][c][f][m][a] =
                                    (TH1F*) hist_comm->Clone(
                                        name_comm.str().c_str());
                        }

                    }
                }
            }
        }
    }
    input_file.Close();
}

void MainWindow::on_pushButton_load_flood_clicked()
{
    TFile input_file(ui->lineEdit_flood->text().toAscii());
    if (input_file.IsZombie()) {
        QMessageBox::warning(this, "Flood Load Failed",
                             "Failed to open flood file: " + filename_flood);
        return;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        stringstream name_flood;
                        name_flood << "floods[" << p << "]["
                                  << c << "][" << f << "]["
                                  << m << "][" << a << "]";

                        TH2F * flood = (TH2F*) input_file.Get(
                                    name_flood.str().c_str());
                        if (flood) {
                            floods[p][c][f][m][a] =
                                    (TH2F*) flood->Clone(
                                        name_flood.str().c_str());
                        }
                    }
                }
            }
        }
    }
    input_file.Close();
}

void MainWindow::on_pushButton_load_graph_clicked()
{
    TFile input_file(ui->lineEdit_flood->text().toAscii());
    if (input_file.IsZombie()) {
        QMessageBox::warning(this, "Graph Load Failed",
                             "Failed to open graph file: " + filename_graph);
        return;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        stringstream name_graph;
                        name_graph << "peaks[" << p << "]["
                                  << c << "][" << f << "]["
                                  << m << "][" << a << "]";

                        TGraph * graph = (TGraph*) input_file.Get(
                                    name_graph.str().c_str());
                        if (graph) {
                            graphs[p][c][f][m][a] =
                                    (TGraph*) graph->Clone(
                                        name_graph.str().c_str());
                        }
                    }
                }
            }
        }
    }
    input_file.Close();
}

void MainWindow::on_pushButton_load_crystal_clicked()
{
    TFile input_file(ui->lineEdit_apd->text().toAscii());
    if (input_file.IsZombie()) {
        QMessageBox::warning(this, "APD Load Failed",
                             "Failed to open apd file: " + filename_apd);
        return;
    }
    for (int p = 0; p < config.panels_per_system; p++) {
        for (int c = 0; c < config.cartridges_per_panel; c++) {
            for (int f = 0; f < config.fins_per_cartridge; f++) {
                for (int m = 0; m < config.modules_per_fin; m++) {
                    for (int a = 0; a < config.apds_per_module; a++) {
                        for (int x = 0; x < config.crystals_per_apd; x++) {
                            stringstream name_spat;
                            name_spat << "E[" << p << "]["
                                      << c << "][" << f << "]["
                                      << m << "][" << a << "][" << x << "]";
                            stringstream name_comm;
                            name_comm << "E_com[" << p << "]["
                                      << c << "][" << f << "]["
                                      << m << "][" << a << "][" << x << "]";

                            TH1F * hist_spat = (TH1F*) input_file.Get(
                                        name_spat.str().c_str());
                            if (hist_spat) {
                                hists_crystal_spat[p][c][f][m][a][x] =
                                        (TH1F*) hist_spat->Clone(
                                            name_spat.str().c_str());
                            }
                            TH1F * hist_comm = (TH1F*) input_file.Get(
                                        name_comm.str().c_str());
                            if (hist_spat) {
                                hists_crystal_comm[p][c][f][m][a][x] =
                                        (TH1F*) hist_comm->Clone(
                                            name_comm.str().c_str());
                            }
                        }
                    }
                }
            }
        }
    }
    input_file.Close();
}
