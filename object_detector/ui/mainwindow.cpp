#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "../algo/cvmat_to_pixmap.hpp"
#include "../utility/global.hpp"
#include "setting_window.hpp"

#include <ocv_libs/qt/mat_and_qimage.hpp>

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>

#include <tiny_cnn/tiny_cnn.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    img_index_(0),
    label_has_focus_(false),
    setting_win_(new setting_window(this))
{
    ui->setupUi(this);

    showMaximized();
    maximum_size_ = ui->graphicsViewImg->size();
    ui->graphicsViewImg->set_enable_focus(false);
    ui->comboBoxTasks->addItems(global::algo_list);

    mat_to_pixmap_ =
            std::make_unique<cvmat_to_pixmap>(maximum_size_);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenFolder_triggered()
{
    QFileDialog dialog(nullptr, tr("Open File"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontResolveSymlinks);
    QStringList filters;
    filters << "Image files (*.png *.bmp *.jpg)";
    dialog.setNameFilters(filters);
    if(dialog.exec()){
        file_names_ = dialog.selectedFiles();
        if(!file_names_.isEmpty()){
            ui->graphicsViewImg->set_pixmap(QPixmap(file_names_[0]));
            ui->graphicsViewImg->fit_in_view();
            label_has_focus_ = false;
            ui->graphicsViewImg->clear_rubber_band();
            img_index_ = 0;
            has_image(true);
        }
    }
}

void MainWindow::on_actionStart_triggered()
{    
    //algo_dispatch_->run(file_names_[img_index_],
    //                    setting_win_,
    //                    ui->label_img->regions(),
    //                    ui->label_img->pixmap()->size());
}

void MainWindow::on_actionStop_triggered()
{

}

void MainWindow::on_actionSetting_triggered()
{
    setting_win_->exec();
}

void MainWindow::on_actionClearAll_triggered()
{
    file_names_.clear();
}

void MainWindow::on_actionRegion_triggered()
{
    if(label_has_focus_){
        ui->actionRegion->setIcon(QIcon(":/pics/layer.png"));
        ui->graphicsViewImg->set_enable_focus(false);
        ui->graphicsViewImg->clear_rubber_band();
    }else{
        ui->actionRegion->setIcon(QIcon(":/pics/layer_delete.png"));
        ui->graphicsViewImg->set_enable_focus(true);
    }
    label_has_focus_ = !label_has_focus_;
}

void MainWindow::has_image(bool value)
{
    ui->actionRegion->setEnabled(value);
    ui->actionStart->setEnabled(value);
}
