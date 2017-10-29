# Kocr

Kocr is a graphical interface for Tesseract with support for HTML and searchable PDF output on GNU/Linux, Windows, and possibly MacOS.

# Requirements

Kocr works with Qt 5.6 graphical libraries.

Kocr uses the following programs:
* Tesseract
* Ghostscript
* Imagemagick

Please, check the releases section for binary packages if you want to try it. On GNU/Linux, dependencies must be installed using your system's packaging tools (E.G.: On Debian it's apt-get). On Windows, they must be placed on C:\Programs or in subfolders of kocr directory.

# How to use Kocr

Here's what you would likely do with Kocr:
* Load a few pages as images (png, jpeg, tiff, bmp, gif) or from PDFs (this could take a few minutes for pages extraction)
* Check pages clicking on them and eventually removing them if they are not needed. It's possibile to select multiple pages with Shift or Ctrl, to delete them.
* Move pages on the list to change their order, just by dragging and dropping them
* Choose your desired output format: Plain text is the default, but you can also get HTML (which gives you formatted text that you can also open with LibreOffice Writer), od searchable PDF (which is a PDF made out of the page images you provided with the recognized text embedded so you can select it)
* The program will ask you if you want to save the output to a file, and try to display it

Please, take note that the OCR process may take several minutes. The program may look like it's freezing, but it's not: just wait and don't click on the program window until it presents you the result.

# Credits

Kocr has been created by Luca Tringali.
