#ifndef KOCRMAINWINDOW_H
#define KOCRMAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QFileInfoList>
#include <QVariant>
#include <QSize>
#include <QProcess>
#include <QByteArray>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QCoreApplication>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QImage>
#include <QUrl>
#include <QTransform>
#include <QDesktopServices>
#include <QDebug>

namespace Ui {
class kocrMainWindow;
}

class kocrMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit kocrMainWindow(QWidget *parent = 0);
    ~kocrMainWindow();
    QString osName();
    QString tesseractocr(QString imagepath, QString command = "", QString language = "", bool html = false, QString pdffile = "");
    QString cuneiformocr(QString imagepath, QString command = "", QString language = "", bool html = false, QString pdffile = "");
    void rotateimg(QString imgpath, double angle);
    void findocr();
    QString dpi;

private slots:
    void on_importimg_clicked();

    void on_pushButton_2_clicked();

    void on_ocrengine_currentIndexChanged(const QString &arg1);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_importpdf_clicked();

    void on_actionAbout_Kocr_triggered();

    void on_delimage_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_actionAbout_Qt_triggered();

    void on_actionImport_from_images_triggered();

    void on_actionImport_from_PDFs_triggered();

    void on_actionDelete_selected_triggered();

    void on_actionClear_selection_triggered();

    void on_actionNew_triggered();

    void on_actionRun_OCR_triggered();

    void on_actionExit_triggered();

    void on_zoomp_clicked();

    void on_zoomm_clicked();

    void on_actionRotate_selected_90_triggered();

    void on_actionRotate_selected_91_triggered();

private:
    Ui::kocrMainWindow *ui;
    void addimagetolist(QString file);
    void addpdftolist(QString pdfin);
    void displayimage(QListWidgetItem *item, int zoom = 0);
    QString gs;
    QString imconvert;
    QStringList tempfiles;
    QString tesseract;
    QString cuneiform;
    QString startdirectory;
    int previewitem;

};

#endif // KOCRMAINWINDOW_H
