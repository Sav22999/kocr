#include "kocrmainwindow.h"
#include "ui_kocrmainwindow.h"

kocrMainWindow::kocrMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kocrMainWindow)
{
    ui->setupUi(this);

    if (osName() == "linux" || osName() == "unix") {
        QString tesseract = "/usr/bin/tesseract";
        if (QFileInfo(tesseract).exists()) {
            ui->ocrengine->addItem("Tesseract", QVariant(tesseract));
        }

        QString cuneiform = "/usr/bin/cuneiform";
        if (QFileInfo(cuneiform).exists()) {
            ui->ocrengine->addItem("Cuneiform", QVariant(cuneiform));
        }

        if (QFileInfo("/usr/bin/gs").exists()) {
            gs = "/usr/bin/gs";
        }

        if (QFileInfo("/usr/bin/convert").exists()) {
            imconvert = "/usr/bin/convert";
        }

        qDebug() << "Found programs: " << tesseract << cuneiform << gs << imconvert;
    }

    if (osName() == "windows" || osName() == "wince") {
        QString tesseract = QCoreApplication::applicationDirPath() + "/tesseract/tesseract.exe";
        if (QFileInfo(tesseract).exists()) {
            ui->ocrengine->addItem("Tesseract", QVariant(tesseract));
        } else {
            tesseract = "C:/Programs/tesseract/tesseract.exe";
            if (QFileInfo(tesseract).exists()) {
                ui->ocrengine->addItem("Tesseract", QVariant(tesseract));
            }
        }

        QString cuneiform = QCoreApplication::applicationDirPath() + "/cuneiform/cuneiform.exe";
        if (QFileInfo(cuneiform).exists()) {
            ui->ocrengine->addItem("Cuneiform", QVariant(cuneiform));
        }

        if (QFileInfo(QCoreApplication::applicationDirPath() + "/gs/gs.exe").exists()) {
            gs = QCoreApplication::applicationDirPath() + "/gs/gs.exe";
        }

        if (QFileInfo(QCoreApplication::applicationDirPath() + "/imagemagick/convert.exe").exists()) {
            imconvert = QCoreApplication::applicationDirPath() + "/imagemagick/convert.exe";
        }

        qDebug() << "Found programs: " << tesseract << cuneiform << gs << imconvert;
    }

    ui->plaintext->setChecked(true);
    dpi = "300";
}

kocrMainWindow::~kocrMainWindow()
{
    for (int i = 0; i<tempfiles.count(); i++) {
        if (QFileInfo(tempfiles.at(i)).exists()) QFile::remove(tempfiles.at(i));
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

void kocrMainWindow::on_importimg_clicked()
{
    QStringList images = QFileDialog::getOpenFileNames(this, tr("Open images"), QDir::currentPath(), "(*.png);; (*.jpg);; (*.jpeg);; (*.tiff);; (*.tif);; (*.bmp);; (*.gif)");
    for (int i = 0; i<images.count(); i++) {
        addimagetolist(images.at(i));
    }
}

void kocrMainWindow::addimagetolist(QString file)
{
    QListWidgetItem* fileitem = new QListWidgetItem(file);
    fileitem->setIcon(QIcon(file));
    ui->listWidget->setIconSize(QSize(256,256));
    ui->listWidget->addItem(fileitem);
}

void kocrMainWindow::on_ocrengine_currentIndexChanged(const QString &arg1)
{
    QString command = ui->ocrengine->currentData().toString();
    for (int i = 0; i<ui->language->count(); i++) ui->language->removeItem(i);

    //setup language
    if (ui->ocrengine->currentText() == "Tesseract") {
        QStringList arguments;
        arguments << "--list-langs";
        QProcess myProcess;
        myProcess.start(command, arguments);
        if (!myProcess.waitForFinished())
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        qDebug() << result;
        for (int i = 0; i<result.split("\n").count(); i++) {
            if (i>0) {
                if (result.split("\n")[i] != "") ui->language->addItem(result.split("\n")[i]);
            }
        }
        ui->language->setCurrentText("eng");
    }

    if (ui->ocrengine->currentText() == "Cuneiform") {
        QString command = ui->ocrengine->currentData().toString();
    }
}

QString kocrMainWindow::tesseractocr(QString imagepath, QString command, QString language, bool html, QString pdffile)
{
    if (command == "") command = ui->ocrengine->currentData().toString();
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
    if (!myProcess.waitForFinished())
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

QString kocrMainWindow::cuneiformocr(QString imagepath, QString command, QString language, bool html)
{
    //we'll work on this
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
        if (!myProcess.waitForFinished())
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        imagepath = tmpfilename;

        QString pdffile = "";
        if (tmpdir != "") pdffile = tmpdir + "/"+ QString::number(i);

        QString thispage = "";
        if (ui->ocrengine->currentText() == "Tesseract")
            thispage = tesseractocr(imagepath, ui->ocrengine->currentData().toString(), language, html, pdffile);
        if (ui->ocrengine->currentText() == "Cuneiform")
            thispage = cuneiformocr(imagepath, ui->ocrengine->currentData().toString(), language, html);
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
        ui->webView->setHtml(allpages);
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
        if (!myProcess.waitForFinished())
                qDebug() << "Error running subprocess";

        tempfiles << tmpfilename;

        QString finalpdf = "";
        finalpdf = QFileDialog::getSaveFileName(this, tr("Save pdf"), QDir::currentPath(), "*.pdf");
        if (finalpdf == "") {
            finalpdf = tmpfilename;
        } else {
            if (QFileInfo(finalpdf).exists()) QFile::remove(finalpdf);
            QFile::copy(tmpfilename,finalpdf);
        }
        ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        QString content = "<object src=\"" + finalpdf + "\" width=\"800\" height=\"600\" type='application/pdf'><!---Fallback--->Can't display PDF on this system. The file is " + finalpdf + "</object>";
        ui->webView->setHtml(content);
        if (ui->webView->page()->findText("Can't display PDF on this system. The file is " + finalpdf)) QDesktopServices::openUrl(QUrl("file://" + finalpdf, QUrl::TolerantMode));
    }


}



void kocrMainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    if (QFileInfo(item->text()).exists()) {
        QImage image(item->text());
        QGraphicsScene* scene = new QGraphicsScene();
        ui->graphicsView->setScene(scene);
        QGraphicsPixmapItem* pixitem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        scene->addItem(pixitem);
    }
}

void kocrMainWindow::on_importpdf_clicked()
{

    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open pdfs"), QDir::currentPath(), "(*.pdf)");
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
    if (!myProcess.waitForFinished())
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
    QMessageBox::about(this,"About Kocr", "Kocr is a graphical interface for Tesseract with support for HTML and searchable PDF output. \nKocr has been created by Luca Tringali.");
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
