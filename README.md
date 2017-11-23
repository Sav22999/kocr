# Kocr

Kocr is a graphical interface for Tesseract and Cuneiform with support for plain text, HTML formatted text, and searchable PDF output on GNU/Linux, Windows, and possibly MacOS.

# Requirements

Kocr works with Qt 5.6 graphical libraries.

Kocr uses the following programs:
* Tesseract (https://github.com/tesseract-ocr/tesseract) or Cuneiform (https://answers.launchpad.net/cuneiform-linux/+question/266152)
* LibPoppler (https://packages.debian.org/sid/libpoppler-qt5-dev or https://sourceforge.net/projects/poppler-win32/)

Please, check the releases section for binary packages if you want to try it. On GNU/Linux, dependencies must be installed using your system's packaging tools (E.G.: On Debian it's apt-get). On Windows, they must be placed on C:\Programs or in subfolders of kocr directory (as they are in the release zip file).

# How to use Kocr

Here's what you would likely do with Kocr:
* Load a few pages as images (png, jpeg, tiff, bmp, gif) or from PDFs (this could take a few minutes for pages extraction)
* Check pages clicking on them and eventually removing them if they are not needed. It's possibile to select multiple pages with Shift or Ctrl, to delete them.
* Move pages on the list to change their order, just by dragging and dropping them
* Choose your desired output format: Plain text is the default, but you can also get HTML (which gives you formatted text that you can also open with LibreOffice Writer), or searchable PDF (which is a PDF made out of the page images you provided with the recognized text embedded so you can select it)
* Wait a few minutes for the OCR to work on the pages
* The program will ask you if you want to save the output to a file, and try to display it

Please, take note that the OCR process may take several minutes. The program may look like it's freezing, but it's not: just wait and don't click on the program window until it presents you the result.

# Credits

Kocr has been created by Luca Tringali and is relased under GNU GPL3. 

Kocr is free software, you are free to fork it and apply as many changes as you like. If you want to contribute to this code, please keep in mind that I'm looking for something that works on both GNU/Linux and Windows and it's as simple as it could possibly be. This program should do one thing, and do it good.

If you use Tesseract, Cuneiform, and Poppler you accept their license. Special thanks goes to the people that wrote those tools for handling images, pdf, and performing OCR.
