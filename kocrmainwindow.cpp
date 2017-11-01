#include "kocrmainwindow.h"
#include "ui_kocrmainwindow.h"

kocrMainWindow::kocrMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kocrMainWindow)
{
    ui->setupUi(this);
    startdirectory = QDir::currentPath();
    setWindowTitle(tr("KOcr"));

    findocr();

    ui->plaintext->setChecked(true);
    dpi = "300";
    previewitem = 0;
}

kocrMainWindow::~kocrMainWindow()
{
    for (int i = 0; i<tempfiles.count(); i++) {
        if (QFileInfo(tempfiles.at(i)).exists()) QFile::remove(tempfiles.at(i));
        qDebug() << "Removing " << tempfiles.at(i);
    }
    delete ui;
}

QString kocrMainWindow::osName()
{
#if defined(Q_OS_ANDROID)
    return QLatin1String("android");
#elif defined(Q_OS_BLACKBERRY)
    return QLatin1String("blackberry");
#elif defined(Q_OS_IOS)
    return QLatin1String("ios");
#elif defined(Q_OS_MAC)
    return QLatin1String("osx");
#elif defined(Q_OS_WINCE)
    return QLatin1String("wince");
#elif defined(Q_OS_WIN)
    return QLatin1String("windows");
#elif defined(Q_OS_LINUX)
    return QLatin1String("linux");
#elif defined(Q_OS_UNIX)
    return QLatin1String("unix");
#else
    return QLatin1String("unknown");
#endif
}

void kocrMainWindow::findocr()
{
    if (osName() == "linux" || osName() == "unix") {
        tesseract = "/usr/bin/tesseract";
        if (QFileInfo(tesseract).exists()) {
            ui->ocrengine->addItem("Tesseract", QVariant("tesseract"));
        } else {
            tesseract = "";
        }

        cuneiform = "/usr/bin/cuneiform";
        if (QFileInfo(cuneiform).exists()) {
            ui->ocrengine->addItem("Cuneiform", QVariant("cuneiform"));
        } else {
            cuneiform = "";
        }

        if (QFileInfo("/usr/bin/gs").exists()) {
            gs = "/usr/bin/gs";
        }

        if (QFileInfo("/usr/bin/convert").exists()) {
            imconvert = "/usr/bin/convert";
        }

    }

    if (osName() == "windows" || osName() == "wince") {
        tesseract = QCoreApplication::applicationDirPath() + "/tesseract/tesseract.exe";
        if (QFileInfo(tesseract).exists()) {
            ui->ocrengine->addItem("Tesseract", QVariant("tesseract"));
        } else {
            tesseract = "C:/Programs/tesseract/tesseract.exe";
            if (QFileInfo(tesseract).exists()) {
                ui->ocrengine->addItem("Tesseract", QVariant("tesseract"));
            } else {
                tesseract = "";
            }
        }

        cuneiform = QCoreApplication::applicationDirPath() + "/cuneiform/cuneiform.exe";
        if (QFileInfo(cuneiform).exists()) {
            ui->ocrengine->addItem("Cuneiform", QVariant("cuneiform"));
        }

        gs = QCoreApplication::applicationDirPath() + "/gs/bin/gswin32c.exe";
        if (!QFileInfo(gs).exists()) {
            gs = "";
        }

        imconvert = QCoreApplication::applicationDirPath() + "/imagemagick/convert.exe";
        if (!QFileInfo(imconvert).exists()) {
            imconvert = "";
        }

    }
    qDebug() << "Found programs: " << tesseract << cuneiform << gs << imconvert;
}

void kocrMainWindow::on_importimg_clicked()
{
    QStringList images = QFileDialog::getOpenFileNames(this, tr("Open images"), startdirectory, "Images (*.png *.jpg *.jpeg *.tiff *.tif *.bmp *.gif)");
    for (int i = 0; i<images.count(); i++) {
        addimagetolist(images.at(i));
    }
}

void kocrMainWindow::addimagetolist(QString file)
{
    QListWidgetItem* fileitem = new QListWidgetItem(file);
    fileitem->setIcon(QIcon(file));
    int previewsize = 128;
    ui->listWidget->setIconSize(QSize(previewsize,previewsize));
    ui->listWidget->addItem(fileitem);
}

void kocrMainWindow::on_ocrengine_currentIndexChanged(const QString &arg1)
{
    ui->language->clear();

    //setup language
    if (ui->ocrengine->currentData().toString() == "tesseract") {
        QString command = tesseract;
        QStringList arguments;
        arguments << "--list-langs";
        //we might need --tessdata-dir  tessdataPath
        //arguments << "--tessdata-dir";
        //arguments << tesseract.mid(0,tesseract.lastIndexOf("/")) + "/tessdata/";
        QProcess myProcess;
        myProcess.start(command, arguments);
        int timeout = -1;//300000; //just use -1 to disable timeout
        qDebug() << "Timeout: " << timeout;
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        qDebug() << result;
        result = result.replace("\r\n","\n");
        for (int i = 0; i<result.split("\n").count(); i++) {
            if (i>0) {
                if (result.split("\n")[i] != "") ui->language->addItem(result.split("\n")[i]);
            }
        }
        ui->language->setCurrentText("eng");
        ui->pdf->setEnabled(true);
    }

    if (ui->ocrengine->currentData().toString() == "cuneiform") {
        QString command = cuneiform;
        QStringList arguments;
        arguments << "-l";
        QProcess myProcess;
        myProcess.start(command, arguments);
        int timeout = -1;//300000; //just use -1 to disable timeout
        qDebug() << "Timeout: " << timeout;
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        qDebug() << result;
        result = result.mid(result.indexOf(":"));
        result = result.replace(".\n","");
        for (int i = 0; i<result.split(" ").count(); i++) {
            if (i>0) {
                if (result.split(" ")[i] != "") ui->language->addItem(result.split(" ")[i]);
            }
        }
        ui->language->setCurrentText("eng");
        ui->pdf->setEnabled(false); //actually automatic pdf generation is not supported
    }
}

QString kocrMainWindow::tesseractocr(QString imagepath, QString command, QString language, bool html, QString pdffile)
{
    if (command == "") command = tesseract;
    if (language == "") language = ui->language->currentText();
    QStringList arguments;
    arguments << "-psm";
    arguments << "3";
    arguments << imagepath;
    QString pdfdir = "";
    QString tmpfilename = "";

    if (pdffile != "") {
        pdfdir = QFileInfo(pdffile).absoluteDir().absolutePath();
        arguments << pdffile;
        arguments << "pdf";
    } else {
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName();
        }
        tfile.close();
        arguments << tmpfilename;
    }

    arguments << "-l";
    arguments << language;
    if (html) {
        arguments << "hocr";
    }

    QProcess myProcess;
    myProcess.start(command, arguments);
    int timeout = 300000; //just use -1 to disable timeout
    qDebug() << "Timeout: " << timeout;
    if (!myProcess.waitForFinished(timeout))
            qDebug() << "Error running subprocess";
    QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
    qDebug() << result;

    QString text = "";
    if (pdffile == "") {
        if (html) {
            tmpfilename += ".hocr";
        } else {
            tmpfilename += ".txt";
        }
        QFile file(tmpfilename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return "";
        QTextStream in(&file);
        while (!in.atEnd()) {
            text += in.readLine();
        }
        file.close();
        QFile::remove(tmpfilename);
    } else {
        text += pdffile + ".pdf";
        tempfiles << pdffile + ".pdf";
    }

    return text;
}

QString kocrMainWindow::cuneiformocr(QString imagepath, QString command, QString language, bool html, QString pdffile)
{
    if (command == "") command = cuneiform;
    if (language == "") language = ui->language->currentText();
    QStringList arguments;
    QString pdfdir = "";
    QString tmpfilename = "";


    if (pdffile != "") {
        pdfdir = QFileInfo(pdffile).absoluteDir().absolutePath();
        arguments << "-f";
        arguments << "hocr";
        arguments << "-o";
        arguments << pdffile + ".hocr";
    } else {
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName();
        }
        tfile.close();
        if (html) {
            arguments << "-f";
            arguments << "hocr";
            tmpfilename += ".hocr";
        } else {
            arguments << "-f";
            arguments << "text";
            tmpfilename += ".txt";
        }
        arguments << "-o";
        arguments << tmpfilename;
    }

    arguments << "-l";
    arguments << language;

    arguments << imagepath;


    QProcess myProcess;
    myProcess.start(command, arguments);
    int timeout = 300000; //just use -1 to disable timeout
    qDebug() << "Timeout: " << timeout;
    if (!myProcess.waitForFinished(timeout))
            qDebug() << "Error running subprocess";
    QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
    qDebug() << result;

    QString text = "";
    if (pdffile == "") {
        QFile file(tmpfilename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return "";
        QTextStream in(&file);
        while (!in.atEnd()) {
            text += in.readLine();
        }
        file.close();
        QFile::remove(tmpfilename);
    } else {
        //QString tmphocr = pdffile + ".hocr";
        // TODO: here we should merge tmphocr and imagepath in pdfile+".pdf"   https://exactcode.com/opensource/exactimage/  hocr2pdf -i scan.tiff -o test.pdf < cuneiform-out.hocr
        text += pdffile + ".pdf";
        tempfiles << pdffile + ".pdf";
    }

    return text;
}

void kocrMainWindow::on_pushButton_2_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Please wait"),tr("This is going to take a while. Relax, get a snack and wait for a few minutes."), QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel) {
        return;
    }

    QString language = ui->language->currentText();
    bool html = false;
    if (ui->html->isChecked()) {
        html = true;
    }

    bool pdf = false;
    QString tmpdir = "";
    if (ui->pdf->isChecked()) {
        pdf = true;
        QTemporaryDir dir;
        if (dir.isValid()) {
            tmpdir = dir.path();
            dir.setAutoRemove(false);
        }
        tempfiles << tmpdir;
    }

    QString allpages = "";



    for (int i = 0; i < ui->listWidget->count(); i++) {
        QString imagepath = ui->listWidget->item(i)->text();

        //convert "$f" -background white -flatten +matte "${f%.*}.tiff"
        QStringList arguments;
        arguments << imagepath;
        arguments << "-background";
        arguments << "white";
        arguments << "-flatten";
        arguments << "+matte";

        QString tmpfilename = "";
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName().replace(".","-") + QString(".tiff");
        }
        tfile.close();
        arguments << tmpfilename;

        QProcess myProcess;
        myProcess.start(imconvert, arguments);
        int timeout = 300000; //just use -1 to disable timeout
        qDebug() << "Timeout: " << timeout;
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        imagepath = tmpfilename;

        QString pdffile = "";
        if (tmpdir != "") pdffile = tmpdir + "/"+ QString::number(i);

        QString thispage = "";
        if (ui->ocrengine->currentText() == "Tesseract")
            thispage = tesseractocr(imagepath, tesseract, language, html, pdffile);
        if (ui->ocrengine->currentText() == "Cuneiform")
            thispage = cuneiformocr(imagepath, cuneiform, language, html);
        qDebug() << thispage;

        if (html) {
            allpages += thispage + "\n</br></hr>";
        } else if (pdf) {
            allpages += thispage + "|";
        } else {
            allpages += thispage + "\n";
        }

        QFile::remove(tmpfilename);

    }

    if (!pdf){
        ui->result->setHtml(allpages);
        QString finalfile = "";
        if (html) {
            finalfile = QFileDialog::getSaveFileName(this, tr("Save html"), startdirectory, ".html (*.html)");
        } else {
            finalfile = QFileDialog::getSaveFileName(this, tr("Save text"), startdirectory, ".txt (*.txt)");
        }
        if (finalfile != "") {
            QFile file(finalfile);
            if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                QTextStream stream(&file);
                stream.setCodec("UTF-8");
                stream << allpages;
            }
        }
    } else {
        //merge pdfs if necessary
        //gs -dCompatibilityLevel=1.4 -dNOPAUSE -dQUIET -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=${name}.searchable.pdf *.tmp.pdf
        QStringList arguments;
        arguments << "-dCompatibilityLevel=1.4";
        arguments << "-dNOPAUSE";
        arguments << "-dQUIET";
        arguments << "-dBATCH";
        arguments << "-dNOPAUSE";
        arguments << "-q";
        arguments << "-sDEVICE=pdfwrite";
        QString tmpfilename = "";
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName() + QString(".pdf");
        }
        tfile.close();
        QString out = "-sOutputFile=";
        out += tmpfilename;
        arguments << out;

        for (int i = 0; i<allpages.split("|").count(); i++) {
             QString inp = "";
             inp = allpages.split("|").at(i);
             if (QFileInfo(inp).exists()) arguments << inp;
         }

        QProcess myProcess;
        myProcess.start(gs, arguments);
        int timeout = 300000; //just use -1 to disable timeout
        qDebug() << "Timeout: " << timeout;
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";

        tempfiles << tmpfilename;

        QString finalpdf = "";
        finalpdf = QFileDialog::getSaveFileName(this, tr("Save pdf"), startdirectory, "PDF (*.pdf)");
        if (finalpdf == "") {
            finalpdf = tmpfilename;
        } else {
            if (QFileInfo(finalpdf).exists()) QFile::remove(finalpdf);
            QFile::copy(tmpfilename,finalpdf);
        }
        QString content = "<object src=\"" + finalpdf + "\" width=\"800\" height=\"600\" type='application/pdf'><!---Fallback--->Can't display PDF on this system. The file is " + finalpdf + "</object>";
        ui->result->setHtml(content);
        if (osName() == "windows" || osName() == "wince") {
            QDesktopServices::openUrl(QUrl("file:///" + finalpdf, QUrl::TolerantMode));
        }
        if (osName() == "linux" || osName() == "unix") {
            QDesktopServices::openUrl(QUrl("file://" + finalpdf, QUrl::TolerantMode));
        }
    }


}

void kocrMainWindow::displayimage(QListWidgetItem *item, int zoom) {
    double dzoom = zoom+1.0;
    if (zoom == 0) dzoom = 1.0;
    if (zoom < 0) dzoom = -1.0/(zoom-1.0);
    double scale = 1;
    if (QFileInfo(item->text()).exists()) {
        QImage image(item->text());
        scale = (ui->graphicsView->size().width()-30)*dzoom;
        int iscale = scale;
        QGraphicsScene* scene = new QGraphicsScene();
        ui->graphicsView->setScene(scene);
        QGraphicsPixmapItem* pixitem = new QGraphicsPixmapItem(QPixmap::fromImage(image.scaledToWidth(iscale)));
        scene->addItem(pixitem);
    }
}

void kocrMainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    previewitem = ui->listWidget->row(item);
    displayimage(item, ui->horizontalSlider->value());
}

void kocrMainWindow::on_importpdf_clicked()
{

    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open pdfs"), startdirectory, "PDF (*.pdf)");
    for (int i = 0; i<files.count(); i++) {
        addpdftolist(files.at(i));
    }

}


void kocrMainWindow::addpdftolist(QString pdfin)
{
    if (!QFileInfo(pdfin).exists()) return;

    //gs -dNOPAUSE -q -r$dpix$dpi -sDEVICE=tiff32nc -dBATCH -sOutputFile="$name-%04d.tmppage.tiff" "$fullname"
    QStringList arguments;
    arguments << "-dNOPAUSE";
    arguments << "-q";
    arguments << "-r" + dpi + "x" + dpi;
    arguments << "-sDEVICE=tiff32nc";
    arguments << "-dBATCH";
    QString tmpfilename = "";
    QString tmpdir = "";
    QTemporaryDir dir;
    if (dir.isValid()) {
        tmpdir = dir.path();
        dir.setAutoRemove(false);
    }
    tempfiles << tmpdir;
    tmpfilename = tmpdir + QString("/tmppage%04d.tiff");
    QString out = "-sOutputFile=";
    out += tmpfilename;
    arguments << out;

    arguments << pdfin;

    QProcess myProcess;
    myProcess.start(gs, arguments);
    int timeout = 300000; //just use -1 to disable timeout
    qDebug() << "Timeout: " << timeout;
    if (!myProcess.waitForFinished(timeout))
            qDebug() << "Error running subprocess";


    //now we open images
    QDir pdir(tmpdir);
    pdir.setFilter(QDir::Files | QDir::NoSymLinks);
    pdir.setSorting(QDir::Name);
    QFileInfoList list = pdir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        qDebug() << fileInfo.absoluteFilePath();
        tempfiles << fileInfo.absoluteFilePath();
        addimagetolist(fileInfo.absoluteFilePath());
    }

}

void kocrMainWindow::on_actionAbout_Kocr_triggered()
{
    QMessageBox::about(this,"About Kocr", "Kocr is a graphical interface for Tesseract with support for HTML and searchable PDF output. \nKocr has been created by Luca Tringali.\nhttps://github.com/zorbaproject/kocr");
}

void kocrMainWindow::on_delimage_clicked()
{
    if (ui->listWidget->selectedItems().count()>0) {
        foreach(QListWidgetItem * item, ui->listWidget->selectedItems())
        {
            delete ui->listWidget->takeItem(ui->listWidget->row(item));
        }
    }
}

void kocrMainWindow::on_horizontalSlider_valueChanged(int value)
{
    if (previewitem < ui->listWidget->count()) {
        displayimage(ui->listWidget->item(previewitem), value);
    }
}

void kocrMainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void kocrMainWindow::on_actionImport_from_images_triggered()
{
    on_importimg_clicked();
}

void kocrMainWindow::on_actionImport_from_PDFs_triggered()
{
    on_importpdf_clicked();
}

void kocrMainWindow::on_actionDelete_selected_triggered()
{
    on_delimage_clicked();
}

void kocrMainWindow::on_actionClear_selection_triggered()
{
    ui->listWidget->clearSelection();
}

void kocrMainWindow::on_actionNew_triggered()
{
    ui->listWidget->clear();
    QGraphicsScene* scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
}

void kocrMainWindow::on_actionRun_OCR_triggered()
{
    on_pushButton_2_clicked();
}

void kocrMainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Close"),tr("Do you really want to close this program?"), QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel) {
        return;
    }
    QCoreApplication::quit();
}

void kocrMainWindow::on_zoomp_clicked()
{
    int value = ui->horizontalSlider->value() +1;
    ui->horizontalSlider->setValue(value);
}

void kocrMainWindow::on_zoomm_clicked()
{
    int value = ui->horizontalSlider->value() -1;
    ui->horizontalSlider->setValue(value);
}

void kocrMainWindow::on_actionRotate_selected_90_triggered()
{
    double angle = 90.0;
    if (ui->listWidget->selectedItems().count()>0) {
        foreach(QListWidgetItem * item, ui->listWidget->selectedItems())
        {
            rotateimg(item->text(), angle);
            item->setIcon(QIcon(item->text()));
        }
    }
}

void kocrMainWindow::on_actionRotate_selected_91_triggered()
{
    double angle = 270.0;
    if (ui->listWidget->selectedItems().count()>0) {
        foreach(QListWidgetItem * item, ui->listWidget->selectedItems())
        {
            rotateimg(item->text(), angle);
            item->setIcon(QIcon(item->text()));
        }
    }
}

void kocrMainWindow::rotateimg(QString imgpath, double angle)
{
    if (QFileInfo(imgpath).exists()) {
        QImage image(imgpath);
        QTransform rotating;
        rotating.rotate(angle);
        image = image.transformed(rotating);
        image.save(imgpath);
    }
}
