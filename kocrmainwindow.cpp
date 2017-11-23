#include "kocrmainwindow.h"
#include "ui_kocrmainwindow.h"

kocrMainWindow::kocrMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kocrMainWindow)
{
    ui->setupUi(this);
    startdirectory = QDir::currentPath();
    setWindowTitle(tr("KOcr"));
    ui->mainToolBar->hide();

    findocr();

    ui->plaintext->setChecked(true);
    dpi = "300";
    previewitem = 0;
}

kocrMainWindow::~kocrMainWindow()
{
    for (int i = tempfiles.count(); i>0; i--) {
        if (QFileInfo(tempfiles.at(i-1)).exists()) QFile::remove(tempfiles.at(i-1));
        qDebug() << "Removing " << tempfiles.at(i-1);
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

    }

    if (osName() == "windows" || osName() == "wince") {
        tesseract = QCoreApplication::applicationDirPath() + "/tesseract/tesseract.exe";
        if (QFileInfo(tesseract).exists()) {
            ui->ocrengine->addItem("Tesseract", QVariant("tesseract"));
        } else {
            tesseract = "C:/Program Files/tesseract/tesseract.exe";
            if (QFileInfo(tesseract).exists()) {
                ui->ocrengine->addItem("Tesseract", QVariant("tesseract"));
            } else {
                tesseract = "";
            }
        }

        cuneiform = QCoreApplication::applicationDirPath() + "/cuneiform/cuneiform.exe";
        if (QFileInfo(cuneiform).exists()) {
            ui->ocrengine->addItem("Cuneiform", QVariant("cuneiform"));
        } else {
            cuneiform = "C:/Program Files/cuneiform/cuneiform.exe";
            if (QFileInfo(cuneiform).exists()) {
                ui->ocrengine->addItem("Cuneiform", QVariant("cuneiform"));
            } else {
                cuneiform = "";
            }
        }

    }
    qDebug() << "Found programs: " << tesseract << cuneiform;
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
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
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
        if (osName() == "windows" || osName() == "wince") {
            // TODO: check if the library has already been registered
            QString command = "regsvr32";
            QStringList arguments;
            arguments << QFileInfo(cuneiform).absolutePath() + "/Puma.NET/COM Server/APuma.dll";
            QProcess myProcess;
            myProcess.setWorkingDirectory(QFileInfo(cuneiform).absolutePath() + "/Puma.NET/COM Server/");
            myProcess.start(command, arguments);
            int timeout = -1;//300000; //just use -1 to disable timeout
            if (!myProcess.waitForFinished(timeout))
                    qDebug() << "Error running subprocess";
            QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
            qDebug() << result;
            result = "Bulgarian, Croatian, Czech, Danish, Digits, Dutch, English, Estonian, French, German, Hungarian, Italian, Lettish, Lithuanian, Polish, Portuguese, Romanian, Russian, RussianEnglish, Serbian, Slovenian, Spanish, Swedish, Ukrainian";
            result = result.replace(",","");
            for (int i = 0; i<result.split(" ").count(); i++) {
                if (i>0) {
                    if (result.split(" ")[i] != "") ui->language->addItem(result.split(" ")[i]);
                }
            }
            ui->language->setCurrentText("English");
        } else {
            QString command = cuneiform;
            QStringList arguments;
            arguments << "-l";
            QProcess myProcess;
            myProcess.start(command, arguments);
            int timeout = -1;//300000; //just use -1 to disable timeout
            if (!myProcess.waitForFinished(timeout))
                    qDebug() << "Error running subprocess";
            QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
            result = result.mid(result.indexOf(":"));
            result = result.replace(".\n","");
            for (int i = 0; i<result.split(" ").count(); i++) {
                if (i>0) {
                    if (result.split(" ")[i] != "") ui->language->addItem(result.split(" ")[i]);
                }
            }
            ui->language->setCurrentText("eng");
        }
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
        if (osName() == "windows" || osName() == "wince") {
            arguments << "-FileFormat=RtfAnsi";
            tmpfilename = pdffile + ".rtf";
        } else {
            arguments << "-f";
            arguments << "hocr";
        }
        if (osName() == "windows" || osName() == "wince") {
            arguments << "-RecognizeToFileName=" + tmpfilename;
            tempfiles << tmpfilename;
        } else {
            arguments << "-o";
            arguments << pdffile + ".hocr";
            tempfiles << pdffile + ".hocr";
        }
    } else {
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName().replace(".","-");
        }
        tfile.close();
        if (html) {
            if (osName() == "windows" || osName() == "wince") {
                arguments << "-FileFormat=RtfAnsi";
                tmpfilename += ".rtf";
            } else {
                arguments << "-f";
                arguments << "hocr";
                tmpfilename += ".hocr";
                qDebug() << tmpfilename;
            }
        } else {
            if (osName() == "windows" || osName() == "wince") {
                arguments << "-FileFormat=TxtAnsi";
            } else {
                arguments << "-f";
                arguments << "text";
            }
            tmpfilename += ".txt";
        }
        if (osName() == "windows" || osName() == "wince") {
            arguments << "-RecognizeToFileName=" + tmpfilename;
        } else {
            arguments << "-o";
            arguments << tmpfilename;
        }
    }

    if (osName() == "windows" || osName() == "wince") {
        arguments << "-Language=" + language;
    } else {
        arguments << "-l";
        arguments << language;
    }

    if (osName() == "windows" || osName() == "wince") {
        arguments << "-LoadImage=" + imagepath + "";
    } else {
        arguments << imagepath;
    }



    QProcess myProcess;
    myProcess.start(command, arguments);
    int timeout = 300000; //just use -1 to disable timeout
    if (!myProcess.waitForFinished(timeout))
            qDebug() << "Error running subprocess";
    QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
    qDebug() << result;

    if (tmpfilename.right(4) == ".rtf") {
        QString command = QFileInfo(cuneiform).absolutePath() + "/rtf2html.exe";
        QStringList arguments;
        arguments << tmpfilename;
        arguments << tmpfilename.mid(0,tmpfilename.length()-4) + ".hocr";
        QProcess myProcess;
        myProcess.start(command, arguments);
        int timeout = -1;//300000; //just use -1 to disable timeout
        if (!myProcess.waitForFinished(timeout))
                qDebug() << "Error running subprocess";
        QString result = QString(myProcess.readAllStandardOutput()) + QString(myProcess.readAllStandardError());
        tmpfilename = tmpfilename.mid(0,tmpfilename.length()-4) + ".hocr";
    }

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
        //QFile::remove(tmpfilename);
    } else {
        // hocr2pdf -i scan.tiff -o test.pdf < cuneiform-out.hocr

        QPdfWriter pdfWriter(pdffile + ".pdf");
        QPainter painter(&pdfWriter);

        if (QFileInfo(imagepath).exists()) {
            QImage image(imagepath);
            image = image.convertToFormat(QImage::Format_RGB32);

            if (osName() == "windows" || osName() == "wince") {

                QFile file(tmpfilename);
                QString hocr = "";
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    return "";
                QTextStream in(&file);
                while (!in.atEnd()) {
                    hocr += in.readLine();
                }
                file.close();

                QTextDocument td;
                td.setHtml(hocr);
                //QRectF newRect(0,0,pdfWriter.width(),pdfWriter.height());
                //painter.drawText(newRect,td.toPlainText());
                td.setTextWidth(pdfWriter.width());
                QFont tfont = td.defaultFont();
                double nsize =(td.defaultFont().pointSizeF()*(pdfWriter.width()/(pdfWriter.logicalDpiX()/3)));
                /*int pixelsWide = 0;
                while (pixelsWide < (pdfWriter.height()/pdfWriter.logicalDpiY())) {
                    tfont.setPixelSize(nsize);
                    QFontMetrics fm(tfont);
                    pixelsWide = fm.boundingRect(td.toPlainText()).height();
                    nsize = nsize+10;
                }*/
                tfont.setPointSizeF(nsize);
                td.setDefaultFont(tfont);
                //td.drawContents(&painter, newRect);
                td.drawContents(&painter);
                painter.drawPixmap(0,0, pdfWriter.width(), pdfWriter.height(), QPixmap::fromImage(image));

            } else {

                QFile file(pdffile + ".hocr");
                QString hocr = "";
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    return "";
                QTextStream in(&file);
                while (!in.atEnd()) {
                    hocr += in.readLine();
                }
                file.close();

                int textpos = hocr.indexOf("bbox 0 0 ")+9;
                double totalw = hocr.mid(textpos,hocr.indexOf(" ",textpos)-textpos).toDouble();
                textpos = hocr.indexOf(" ",textpos)+1;
                double totalh = hocr.mid(textpos,hocr.indexOf("'",textpos)-textpos).toDouble();

                double ratiow = pdfWriter.width()/totalw;
                double ratioh = pdfWriter.height()/totalh;

                hocr = hocr.mid(hocr.indexOf("<span"));
                hocr = hocr.replace("<b>","");
                hocr = hocr.replace("</b>","");

                while (hocr.indexOf("bbox ")>0) {
                    int textpos = hocr.indexOf("bbox ")+5;
                    double tx = hocr.mid(textpos,hocr.indexOf(" ",textpos)-textpos).toDouble();
                    textpos = hocr.indexOf(" ",textpos)+1;
                    double ty = hocr.mid(textpos,hocr.indexOf(" ",textpos)-textpos).toDouble();
                    textpos = hocr.indexOf(" ",textpos)+1;
                    double tw = hocr.mid(textpos,hocr.indexOf(" ",textpos)-textpos).toDouble()-tx;
                    textpos = hocr.indexOf(" ",textpos)+1;
                    double th = hocr.mid(textpos,hocr.indexOf('"',textpos)-textpos).toDouble()-ty;
                    textpos = hocr.indexOf('>',textpos)+1;
                    QString curtext = hocr.mid(textpos,hocr.indexOf('<',textpos)-textpos);
                    textpos = hocr.indexOf('<',textpos)+1;
                    hocr = hocr.mid(textpos);
                    QRectF newRect(tx*ratiow,ty*ratioh,tw*ratiow,th*ratioh);
                    painter.drawText(newRect, curtext);
                }
                painter.drawPixmap(0,0, pdfWriter.width(), pdfWriter.height(), QPixmap::fromImage(image));
            }

        }

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
        QString tmpfilename = "";
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName().replace(".","-") + QString(".tiff");
        }
        tfile.close();
        tempfiles << tmpfilename;

        QImage image(imagepath);
        QImage image2(image.size(), QImage::Format_RGB32);
        image2.fill(QColor(Qt::white).rgb());
        QPainter painter(&image2);
        painter.drawImage(0, 0, image);
        image2.save(tmpfilename);

        imagepath = tmpfilename;

        QString pdffile = "";
        if (tmpdir != "") pdffile = tmpdir + "/"+ QString::number(i);

        QString thispage = "";
        if (ui->ocrengine->currentText() == "Tesseract")
            thispage = tesseractocr(imagepath, tesseract, language, html, pdffile);
        if (ui->ocrengine->currentText() == "Cuneiform")
            thispage = cuneiformocr(imagepath, cuneiform, language, html, pdffile);

        if (html) {
            allpages += thispage + "\n</br></hr>";
        } else if (pdf) {
            allpages += thispage + "|";
        } else {
            allpages += thispage + "\n";
        }

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
        QString tmpfilename = "";
        QTemporaryFile tfile;
        if (tfile.open()) {
            tmpfilename = tfile.fileName() + QString(".pdf");
        }
        tfile.close();

        //QFile fpdf(tmpfilename);
         //fpdf.open(QIODevice::WriteOnly);
         //QPdfWriter pdfWriter(&fpdf);
        QPdfWriter pdfWriter(tmpfilename);
         QPainter painter(&pdfWriter);

        for (int i = 0; i<allpages.split("|").count(); i++) {
             QString inp = "";
             inp = allpages.split("|").at(i);
             if (QFileInfo(inp).exists()) {

                 if (i>0) pdfWriter.newPage();
                 Poppler::Document* document = Poppler::Document::load(inp);
                 //document->setRenderBackend(Poppler::Document::RenderBackend::ArthurBackend); //you only need this if you want to use pdfPage->renderToPainter(&painter);
                 if (!document || document->isLocked()) return;
                 for (int t = 0; t<document->numPages(); t++) {
                    Poppler::Page* pdfPage = document->page(t);  // Document starts at page 0
                    if (pdfPage == 0) break;
                    //pdfPage->renderToPainter(&painter,dpi.toInt(),dpi.toInt());
                    QList<Poppler::TextBox*> tb = pdfPage->textList();
                    for (int n = 0; n<tb.count(); n++) {
                        QRectF origRect = tb.at(n)->boundingBox();
                        double ratiow = pdfWriter.width()/pdfPage->pageSizeF().width();
                        double ratioh = pdfWriter.height()/pdfPage->pageSizeF().height();
                        QRectF newRect(origRect.x()*ratiow,origRect.y()*ratioh,origRect.width()*ratiow,origRect.height()*ratioh);
                        painter.drawText(newRect, tb.at(n)->text());
                    }
                    painter.drawPixmap(0,0, pdfWriter.width(), pdfWriter.height(), QPixmap::fromImage(pdfPage->renderToImage(dpi.toInt(),dpi.toInt())));
                    //QTextDocument td;
                    //td.setHtml(hocr);
                    //td.drawContents(&painter);
                 }
                 delete document;
             }
         }

        //fpdf.close();
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
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Please wait"),tr("This is going to take a few minutes. Meanwhile, you can have an espresso coffee. Maybe also some cookies, if there are a lot of pages to extract from PDFs."), QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel) {
        return;
    }
    for (int i = 0; i<files.count(); i++) {
        addpdftolist(files.at(i));
    }

}


void kocrMainWindow::addpdftolist(QString pdfin)
{
    if (!QFileInfo(pdfin).exists()) return;

    //gs -dNOPAUSE -q -r$dpix$dpi -sDEVICE=tiff32nc -dBATCH -sOutputFile="$name-%04d.tmppage.tiff" "$fullname"
    QString tmpdir = "";
    QTemporaryDir dir;
    if (dir.isValid()) {
        tmpdir = dir.path();
        dir.setAutoRemove(false);
    }
    tempfiles << tmpdir;

    Poppler::Document* document = Poppler::Document::load(pdfin);
    if (!document || document->isLocked()) return;

    for (int i = 0; i<document->numPages(); i++) {
        Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
        if (pdfPage == 0) break;
        QImage tmpimage = pdfPage->renderToImage(dpi.toInt(), dpi.toInt());
        QString tmpfilename = "";
        tmpfilename = tmpdir + QString("/tmppage") + QString::number(i).rightJustified(4, '0') + QString(".tiff");
        tmpimage.save(tmpfilename);
        qDebug() << tmpfilename;
        delete pdfPage;
    }
    delete document;

    //now we open images
    QDir pdir(tmpdir);
    pdir.setFilter(QDir::Files | QDir::NoSymLinks);
    pdir.setSorting(QDir::Name);
    QFileInfoList list = pdir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        tempfiles << fileInfo.absoluteFilePath();
        addimagetolist(fileInfo.absoluteFilePath());
    }

}

void kocrMainWindow::on_actionAbout_Kocr_triggered()
{
    QMessageBox::about(this,"About Kocr", "Kocr is a graphical interface for Tesseract and Cuneiform with support for HTML and searchable PDF output. \nKocr has been created by Luca Tringali.\nhttps://github.com/zorbaproject/kocr");
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
    //reload preview
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
    //reload preview
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
