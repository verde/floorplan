#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpicture.h>
#include <QtSvg>

class MyMainWindow:public QWidget
{
	public:
		MyMainWindow(const char *fname);
	
	protected:
		void paintEvent(QPaintEvent *);
	
	private:
		QPainter *paint;
		QPicture *pic;
};

MyMainWindow::MyMainWindow(const char *fname)
{
	resize(200,200);
	paint = new QPainter();
	pic = new QPicture();
	pic->load(fname, "svg");
}

void MyMainWindow::paintEvent(QPaintEvent *)
{
	paint->begin(this);
	paint->setWindow(pic->boundingRect());
	paint->drawPicture(0, 0, *pic);
	paint->end();
}

int main(int argc, char **argv)
{
	QApplication a(argc,argv);

	if (argc <= 1)
	{
		return 0;
	}
	
	MyMainWindow w(argv[1]);

	a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
	w.show();
	a.exec();

	return 1;
}

