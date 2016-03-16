#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TQtWidget.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TStyle.h>
#include <TLine.h>
#include <jsoncpp/json/json.h>

using namespace std;

namespace {
void rollSpinBoxes(QSpinBox *changed, QSpinBox *parent)
{
    if (changed->value() == changed->minimum()) {
        if (parent->value() > parent->minimum()) {
            parent->setValue(parent->value() - 1);
            changed->setValue(changed->maximum() - 1);
        } else {
            changed->setValue(changed->value() + 1);
        }
    } else if (changed->value() == changed->maximum()) {
        if (parent->value() < parent->maximum()) {
            parent->setValue(parent->value() + 1);
            changed->setValue(changed->minimum() + 1);
        } else {
            changed->setValue(changed->value() - 1);
        }
    }
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    current_panel(0),
    current_cartridge(0),
    current_fin(0),
    current_module(0),
    current_apd(0)
{
    ui->setupUi(this);


    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_Open()));

    setButtonsEnabled(false);


    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    // Setup the color scale that we've come to expect from floods
    Double_t stops_even[NRGBs] = {0.00, 0.25, 0.50, 0.75, 1.00};
    Double_t inv_blue_green[NRGBs] = {1.00, 0.75, 0.56, 0.00, 0.00};
    Double_t inv_blue_blue[NRGBs]  = {1.00, 1.00, 1.00, 0.80, 0.50};
    Double_t inv_blue_red[NRGBs]   = {0.88, 0.00, 0.12, 0.00, 0.00};
    TColor::CreateGradientColorTable(
                NRGBs, stops_even,
                inv_blue_red, inv_blue_green, inv_blue_blue,
                NCont);

    ui->spinBox_panel->setMinimum(0);
    ui->spinBox_panel->setMaximum(0);
    ui->spinBox_cartridge->setMinimum(0);
    ui->spinBox_cartridge->setMaximum(0);
    ui->spinBox_fin->setMinimum(0);
    ui->spinBox_fin->setMaximum(0);
    ui->spinBox_module->setMinimum(0);
    ui->spinBox_module->setMaximum(0);
    ui->spinBox_apd->setMinimum(0);
    ui->spinBox_apd->setMaximum(0);

    line_gain_spat = new TLine;
    line_gain_comm = new TLine;
    line_gain_spat->SetLineColor(kGreen);
    line_gain_comm->SetLineColor(kGreen);
    line_gain_spat->SetLineWidth(2);
    line_gain_comm->SetLineWidth(2);

    ui->widget_root_00->EnableSignalEvents(kButton1Down);
    ui->widget_root_01->EnableSignalEvents(kButton1Down);
    ui->widget_root_00->setCursor(0);
    ui->widget_root_01->setCursor(0);

    connect(ui->widget_root_00,
            SIGNAL(RootEventProcessed(TObject*,uint,TCanvas*)),
            this,
            SLOT(canvasEvent(TObject*,uint,TCanvas*)));
    connect(ui->widget_root_01,
            SIGNAL(RootEventProcessed(TObject*,uint,TCanvas*)),
            this,
            SLOT(canvasEvent(TObject*,uint,TCanvas*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlots()
{
    bool in_bounds = config.inBoundsPCFMA(
                current_panel, current_cartridge,
                current_fin, current_module, current_apd);
    if (!in_bounds) {
        return;
    }

    ModuleConfig & mod_config = config.module_configs[current_panel]
            [current_cartridge][current_fin][current_module];
    ApdConfig & apd_config = mod_config.apd_configs[current_apd];

    // (ROOT BUG) Plots need this to keep from overwriting themselves. For more
    // information, see:
    // https://root.cern.ch/phpBB3/viewtopic.php?f=3&t=17518
    // gVirtualX->SetFillColor(1);

    TH2F * flood = floods[current_panel][current_cartridge]
                         [current_fin][current_module][current_apd];

    TGraph * graph = graphs[current_panel][current_cartridge]
                           [current_fin][current_module][current_apd];

    if (flood) {
        TCanvas * canvas = ui->widget_root_00->GetCanvas();
        canvas->cd();
        gVirtualX->SetFillColor(1);
        flood->Draw("colz");
        canvas->Update();

        canvas = ui->widget_root_10->GetCanvas();
        canvas->cd();
        gVirtualX->SetFillColor(1);
        flood->Draw("colz");
        canvas->Update();
    }

    if (graph) {
        TCanvas * canvas = ui->widget_root_10->GetCanvas();
        canvas->cd();
        graph->Draw();
        canvas->Update();
    }

    TH1F * hist_spat = hists_apd_spat[current_panel][current_cartridge]
                                     [current_fin][current_module][current_apd];
    if (hist_spat) {
        TCanvas * canvas = ui->widget_root_01->GetCanvas();
        canvas->cd();
        gVirtualX->SetFillColor(1);
        hist_spat->Draw();
        canvas->Update();
        line_gain_spat->DrawLine(apd_config.gain_spat, 0,
                                 apd_config.gain_spat, canvas->GetUymax());
        canvas->Update();
    }


    TH1F * hist_comm = hists_apd_comm[current_panel][current_cartridge]
                                     [current_fin][current_module][current_apd];
    if (hist_comm) {
        TCanvas * canvas = ui->widget_root_11->GetCanvas();
        canvas->cd();
        gVirtualX->SetFillColor(1);
        hist_comm->Draw();
        canvas->Update();
        line_gain_comm->DrawLine(apd_config.gain_comm, 0,
                                 apd_config.gain_comm, canvas->GetUymax());
        canvas->Update();
    }
}

void MainWindow::canvasEvent(TObject *obj, unsigned int event, TCanvas * c)
{
    // Take from: ftp://root.cern.ch/root/doc/26ROOTandQt.pdf
    TQtWidget * clicked = (TQtWidget*) sender();
    if (clicked == ui->widget_root_00 && event == kButton1Down) {
        clicked->GetCanvas()->cd();
        float x = gPad->AbsPixeltoX(clicked->GetEventX());
        float y = gPad->AbsPixeltoY(clicked->GetEventY());
        cout << "flood: " << x << " " << y << endl;
    } else if (clicked == ui->widget_root_01 && event == kButton1Down) {
        clicked->GetCanvas()->cd();
        float x = gPad->AbsPixeltoX(clicked->GetEventX());
        cout << "hist : " << x << endl;
    }
}

void MainWindow::action_Open()
{
    QString full_filename_config = QFileDialog::getOpenFileName(
                this,
                "Select Calibration Config File",
                last_directory,
                "Json File (*.json);; All Files (*)");


    QFileInfo config_file_info(full_filename_config);
    filename_calibration_config = config_file_info.completeBaseName();
    filepath_calibration_config = config_file_info.dir().canonicalPath();

    last_directory = filepath_calibration_config;



    ifstream json_in(full_filename_config.toStdString().c_str());
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(json_in, root);
    json_in.close();
    if (!parsingSuccessful) {
        cerr << reader.getFormatedErrorMessages() << endl;
        return;
    }

    if (!root["config_file"].isString()) {
        return;
    }

    if (!root["ped_file"].isString()) {
        return;
    }

    if (!root["pp_file"].isString()) {
        return;
    }

    if (!root["loc_file"].isString()) {
        return;
    }

    if (!root["cal_file"].isString()) {
        return;
    }

    if (!root["pp_root_file"].isString()) {
        return;
    }

    if (!root["flood_root_file"].isString()) {
        return;
    }

    if (!root["graphs_root_file"].isString()) {
        return;
    }

    if (!root["crystal_energy_root_file"].isString()) {
        return;
    }

    filepath_config = filepath_calibration_config;
    filepath_ped = filepath_calibration_config;
    filepath_pp = filepath_calibration_config;
    filepath_loc = filepath_calibration_config;
    filepath_cal = filepath_calibration_config;
    filepath_apd = filepath_calibration_config;
    filepath_flood = filepath_calibration_config;
    filepath_graph = filepath_calibration_config;
    filepath_crystal = filepath_calibration_config;

    filename_config = QString(root["config_file"].asCString());
    filename_ped = QString(root["ped_file"].asCString());
    filename_pp = QString(root["pp_file"].asCString());
    filename_loc = QString(root["loc_file"].asCString());
    filename_cal = QString(root["cal_file"].asCString());
    filename_apd = QString(root["pp_root_file"].asCString());
    filename_flood = QString(root["flood_root_file"].asCString());
    filename_graph = QString(root["graphs_root_file"].asCString());
    filename_crystal = QString(root["crystal_energy_root_file"].asCString());


    ui->lineEdit_config->setText(filepath_config + "/" + filename_config);
    ui->lineEdit_ped->setText(filepath_ped + "/" + filename_ped);
    ui->lineEdit_pp->setText(filepath_pp + "/" + filename_pp);
    ui->lineEdit_loc->setText(filepath_loc + "/" + filename_loc);
    ui->lineEdit_cal->setText(filepath_cal + "/" + filename_cal);
    ui->lineEdit_apd->setText(filepath_apd + "/" + filename_apd);
    ui->lineEdit_flood->setText(filepath_flood + "/" + filename_flood);
    ui->lineEdit_graph->setText(filepath_graph + "/" + filename_graph);
    ui->lineEdit_crystal->setText(filepath_crystal + "/" + filename_crystal);

    // Load all of the files that exist
    QFile file_info_config(ui->lineEdit_config->text());
    if (file_info_config.exists()) {
        on_pushButton_load_config_clicked();
    }

    QFile file_info_ped(ui->lineEdit_ped->text());
    if (file_info_ped.exists()) {
        on_pushButton_load_ped_clicked();
    }

    QFile file_info_pp(ui->lineEdit_pp->text());
    if (file_info_pp.exists()) {
        on_pushButton_load_pp_clicked();
    }

    QFile file_info_loc(ui->lineEdit_loc->text());
    if (file_info_loc.exists()) {
        on_pushButton_load_loc_clicked();
    }

    QFile file_info_cal(ui->lineEdit_cal->text());
    if (file_info_cal.exists()) {
        on_pushButton_load_cal_clicked();
    }

    QFile file_info_apd(ui->lineEdit_apd->text());
    if (file_info_apd.exists()) {
        on_pushButton_load_apd_clicked();
    }

    QFile file_info_flood(ui->lineEdit_flood->text());
    if (file_info_flood.exists()) {
        on_pushButton_load_flood_clicked();
    }

    QFile file_info_graph(ui->lineEdit_graph->text());
    if (file_info_graph.exists()) {
        on_pushButton_load_graph_clicked();
    }

//    QFile file_info_crystal(ui->lineEdit_crystal->text());
//    if (file_info_crystal.exists()) {
//        on_pushButton_load_crystal_clicked();
//    }

    updatePlots();
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
    ui->pushButton_load_cal->setEnabled(val);
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
    filename_config = config_file_info.completeBaseName();
    filepath_config = config_file_info.dir().canonicalPath();

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
    filename_ped = ped_file_info.completeBaseName();
    filepath_ped = ped_file_info.dir().canonicalPath();

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
    filename_pp = pp_file_info.completeBaseName();
    filepath_pp = pp_file_info.dir().canonicalPath();

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
    filename_loc = loc_file_info.completeBaseName();
    filepath_loc = loc_file_info.dir().canonicalPath();

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
    filename_cal = cal_file_info.completeBaseName();
    filepath_cal = cal_file_info.dir().canonicalPath();

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
    filename_apd = apd_file_info.completeBaseName();
    filepath_apd = apd_file_info.dir().canonicalPath();

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
    filename_flood = flood_file_info.completeBaseName();
    filepath_flood = flood_file_info.dir().canonicalPath();

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
    filename_graph = graph_file_info.completeBaseName();
    filepath_graph = graph_file_info.dir().canonicalPath();

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
    filename_ped = ped_file_info.completeBaseName();
    filepath_ped = ped_file_info.dir().canonicalPath();

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


    ui->spinBox_panel->setMinimum(0);
    ui->spinBox_panel->setMaximum(config.panels_per_system - 1);
    ui->spinBox_cartridge->setMinimum(-1);
    ui->spinBox_cartridge->setMaximum(config.cartridges_per_panel);
    ui->spinBox_fin->setMinimum(-1);
    ui->spinBox_fin->setMaximum(config.fins_per_cartridge);
    ui->spinBox_module->setMinimum(-1);
    ui->spinBox_module->setMaximum(config.modules_per_fin);
    ui->spinBox_apd->setMinimum(-1);
    ui->spinBox_apd->setMaximum(config.apds_per_module);

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
    TFile input_file(ui->lineEdit_apd->text().toStdString().c_str());
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

                        input_file.GetObject(
                                    name_spat.str().c_str(),
                                    hists_apd_spat[p][c][f][m][a]);
                        if (hists_apd_spat[p][c][f][m][a]) {
                            hists_apd_spat[p][c][f][m][a]->SetDirectory(0);
                        }

                        input_file.GetObject(
                                    name_comm.str().c_str(),
                                    hists_apd_comm[p][c][f][m][a]);
                        if (hists_apd_comm[p][c][f][m][a]) {
                            hists_apd_comm[p][c][f][m][a]->SetDirectory(0);
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
    TFile input_file(ui->lineEdit_flood->text().toStdString().c_str());
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

                        input_file.GetObject(
                                    name_flood.str().c_str(),
                                    floods[p][c][f][m][a]);
                        if (floods[p][c][f][m][a]) {
                            floods[p][c][f][m][a]->SetDirectory(0);
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
    TFile input_file(ui->lineEdit_graph->text().toStdString().c_str());
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
                        input_file.GetObject(
                                    name_graph.str().c_str(),
                                    graphs[p][c][f][m][a]);
                    }
                }
            }
        }
    }
    input_file.Close();
}

void MainWindow::on_pushButton_load_crystal_clicked()
{
    TFile input_file(ui->lineEdit_apd->text().toStdString().c_str());
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

                            input_file.GetObject(
                                        name_spat.str().c_str(),
                                        hists_crystal_spat[p][c][f][m][a][x]);
                            if (hists_crystal_spat[p][c][f][m][a][x]) {
                                hists_crystal_spat[p][c][f][m][a][x]->SetDirectory(0);
                            }

                            input_file.GetObject(
                                        name_comm.str().c_str(),
                                        hists_crystal_comm[p][c][f][m][a][x]);
                            if (hists_crystal_comm[p][c][f][m][a][x]) {
                                hists_crystal_comm[p][c][f][m][a][x]->SetDirectory(0);
                            }
                        }
                    }
                }
            }
        }
    }
    input_file.Close();
}

void MainWindow::on_spinBox_panel_valueChanged(int arg1)
{
    current_panel = arg1;
    updatePlots();
}

void MainWindow::on_spinBox_cartridge_valueChanged(int arg1)
{
    // Check edge case where all boxes above this are minned out already
    if (arg1 == ui->spinBox_cartridge->minimum()) {
        if (ui->spinBox_panel->value() == 0) {
            arg1 = 0;
            ui->spinBox_cartridge->setValue(0);
        }
    }

    // Check edge case where all boxes above this are maxed out already
    if (arg1 == ui->spinBox_cartridge->maximum()) {
        if (ui->spinBox_panel->value() == ui->spinBox_panel->maximum()) {
            arg1 -= 1;
            ui->spinBox_cartridge->setValue(arg1);
        }
    }

    current_cartridge = arg1;
    rollSpinBoxes(ui->spinBox_cartridge, ui->spinBox_panel);
    updatePlots();
}

void MainWindow::on_spinBox_fin_valueChanged(int arg1)
{
    // Check edge case where all boxes above this are minned out already
    if (arg1 == ui->spinBox_fin->minimum()) {
        if (ui->spinBox_panel->value() == 0) {
            if (ui->spinBox_cartridge->value() == 0) {
                arg1 = 0;
                ui->spinBox_fin->setValue(0);
            }
        }
    }

    // Check edge case where all boxes above this are maxed out already
    if (arg1 == ui->spinBox_fin->maximum()) {
        if (ui->spinBox_panel->value() == ui->spinBox_panel->maximum()) {
            if (ui->spinBox_cartridge->value() == (ui->spinBox_cartridge->maximum() - 1)) {
                arg1 -= 1;
                ui->spinBox_fin->setValue(arg1);
            }
        }
    }

    current_fin = arg1;
    rollSpinBoxes(ui->spinBox_fin, ui->spinBox_cartridge);
    updatePlots();
}

void MainWindow::on_spinBox_module_valueChanged(int arg1)
{
    // Check edge case where all boxes above this are minned out already
    if (arg1 == ui->spinBox_module->minimum()) {
        if (ui->spinBox_panel->value() == 0) {
            if (ui->spinBox_cartridge->value() == 0) {
                if (ui->spinBox_fin->value() == 0) {
                    ui->spinBox_module->setValue(0);
                }
            }
        }
    }

    // Check edge case where all boxes above this are maxed out already
    if (arg1 == ui->spinBox_module->maximum()) {
        if (ui->spinBox_panel->value() == ui->spinBox_panel->maximum()) {
            if (ui->spinBox_cartridge->value() == (ui->spinBox_cartridge->maximum() - 1)) {
                if (ui->spinBox_fin->value() == (ui->spinBox_fin->maximum() - 1)) {
                    arg1 -= 1;
                    ui->spinBox_module->setValue(arg1);
                }
            }
        }
    }

    current_module = arg1;
    rollSpinBoxes(ui->spinBox_module, ui->spinBox_fin);
    updatePlots();
}

void MainWindow::on_spinBox_apd_valueChanged(int arg1)
{
    // Check edge case where all boxes above this are minned out already
    if (arg1 == ui->spinBox_apd->minimum()) {
        if (ui->spinBox_panel->value() == ui->spinBox_panel->minimum()) {
            if (ui->spinBox_cartridge->value() == (ui->spinBox_cartridge->minimum() + 1)) {
                if (ui->spinBox_fin->value() == (ui->spinBox_fin->minimum() + 1)) {
                    if (ui->spinBox_module->value() == (ui->spinBox_module->minimum() + 1)) {
                        arg1 += 1;
                        ui->spinBox_apd->setValue(arg1);
                    }
                }
            }
        }
    }

    // Check edge case where all boxes above this are maxed out already
    if (arg1 == ui->spinBox_apd->maximum()) {
        if (ui->spinBox_panel->value() == ui->spinBox_panel->maximum()) {
            if (ui->spinBox_cartridge->value() == (ui->spinBox_cartridge->maximum() - 1)) {
                if (ui->spinBox_fin->value() == (ui->spinBox_fin->maximum() - 1)) {
                    if (ui->spinBox_module->value() == (ui->spinBox_module->maximum() - 1)) {
                        arg1 -= 1;
                        ui->spinBox_apd->setValue(arg1);
                    }
                }
            }
        }
    }

    current_apd = arg1;
    rollSpinBoxes(ui->spinBox_apd, ui->spinBox_module);
    updatePlots();
}
