/****************************************************************************
**
** Copyright (C) 2005-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include <QMessageBox>
#include <QXmlStreamAttributes>


#include "dragwidgetgrid.h"

static int max_zoom = 10; //percentage

DragWidgetGrid::DragWidgetGrid(QWidget *parent)
: QGraphicsView (parent)
{
    setMinimumSize(600, 600);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
    setScene(&scene);
    brush = QBrush (Qt::CrossPattern);
    setBackgroundBrush(brush);
	setDragMode(QGraphicsView::RubberBandDrag);
	selectedItem  = NULL;
	m_drawline=false;
	inicial_zoom = height();
}


void DragWidgetGrid::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }

}


void DragWidgetGrid::dragMoveEvent(QDragMoveEvent *event)
{
  
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
  
}

void DragWidgetGrid::wheelEvent(QWheelEvent* event){
  qreal factor = 1.2;
	if (event->delta() < 0)
		factor = 1.0 / factor;
	scale(factor, factor);
	emit zoomChangedSignal(factor);
}


void DragWidgetGrid::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
 
        QPixmap pixmap;
        QPoint offset;
        dataStream >> pixmap >> offset;

        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
		

		QGraphicsPixmapItem * item = scene.addPixmap(pixmap);
		
		item->setPos(item->pos() + mapToScene(event->pos() - offset));
		
		item->setCursor(Qt::ClosedHandCursor);
		
		item->setData(ObjectID,event->mimeData()->text());
		
    } else {
        event->ignore();
    }
}


void DragWidgetGrid::mousePressEvent(QMouseEvent *event)
{
	QGraphicsItem *item;
	bool selected = false;
	if ((item = itemAt(event->pos())) == NULL) {
         qDebug() << "You didn't click on an item.";
     } else {
		qDebug() << "You clicked on an item.";
		selected=true;
     }
	 
	 if(event->button() == Qt::LeftButton) {
		if(!selected)
			m_drawline = true;
		else
			selectedItem = item;	

		drag_start_pos = event->pos();
	}
	else if(selected){
		QMessageBox diag;
		diag.setInformativeText("Tem a certeza que deseja apagar o item em questao?");
 		diag.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		diag.setDefaultButton(QMessageBox::Cancel);
		int ret = diag.exec();
		switch(ret){
			case QMessageBox::Ok:
				scene.removeItem(item);	
			break;
			default:
			break;
		}
	}
}

void DragWidgetGrid::mouseReleaseEvent(QMouseEvent *event){
	//TODO: Fix start drag distance.. tem q ser 1  :/
	if ((event->pos() - drag_start_pos).manhattanLength()
          < QApplication::startDragDistance())
     return;

	if(m_drawline)
	{
		scene.addLine(QLineF(mapToScene(event->pos()),mapToScene(drag_start_pos)),QPen(Qt::SolidLine));
		m_drawline = false;	
	}
	else if(selectedItem){
		selectedItem->setPos(mapToScene(event->pos()));
		selectedItem = NULL;
	}
	drag_start_pos = QPoint(0,0);
}
void DragWidgetGrid::keyReleaseEvent( QKeyEvent * event ){

// 	TODO: ao largar a tecla, emitir os sinais para a mainwindow retirar da status
	switch(event->key()){
		case Qt::Key_Control:
			emit modifierKeyReleasedSignal(Qt::Key_Control);
			break;
		case Qt::Key_Alt:
			emit modifierKeyReleasedSignal(Qt::Key_Alt);
			break;
		case Qt::Key_Meta:
			emit modifierKeyReleasedSignal(Qt::Key_Meta);
			break;
	}
	

}


void DragWidgetGrid::keyPressEvent( QKeyEvent * event ){
	if(selectedItem) {
		/*

		**** TODO: passar o selectedItem para a status bar
		*****TODO: resetTransform ()
		*****TODO: FIXME: posicao dos blocos, setZeValue()
		*****TODO: ajustar com a distancia pecorrida pelo rato
		*****TODO: adicionar o mesmo codigo para o mousepress
		*****TODO: delete key
		*****TODO: undo/redo logic
		*****TODO: adcionar accoes ao qtext do mem. descritiva

		
		accoes:
		* rodar    :  ctrl             + setas/LMB
			-> void QGraphicsItem::rotate ( qreal angle )
			Rotates the current item transformation angle degrees clockwise around its origin.

		* inclinar :  meta             + setas/LMB
			-> void QGraphicsItem::shear ( qreal sh, qreal sv )
			Shears the current item transformation by (sh, sv).
		
		* escalar  :  alt              + setas/LBM
			-> void QGraphicsItem::scale ( qreal sx, qreal sy )
			Scales the current item transformation by (sx, sy) around its origin.
		
		* deslocar :  sem modificador  + setas/LMB
			-> void QGraphicsItem::translate ( qreal dx, qreal dy )
			Translates the current item transformation by (dx, dy).
		
		* inverter : shift
			-> shift + alt + left : reverte o valor extendido
		
		detalhes
		* deslocar :  + 1 pixel*(1/zoom)
			se esta com o zoom out em 2x -> 1
		*/
		QRectF rec = selectedItem->sceneBoundingRect();
		QPointF rec_center = rec.center();
		int mod = QApplication::keyboardModifiers();
		int key_count = event->count();
		switch(event->key()){
			case Qt::Key_Control:
				m_ctrl_flag = true;
				qDebug() << "ctrl1:" << m_ctrl_flag << "c:" << key_count << "m:" << mod;
				emit modifierKeyPressedSignal(Qt::Key_Control);
			break;
			case Qt::Key_Alt:
				emit modifierKeyPressedSignal(Qt::Key_Alt);
			break;
			case Qt::Key_Meta:
				emit modifierKeyPressedSignal(Qt::Key_Meta);
				break;
			case Qt::Key_Right:
				qDebug() << "ctrl2:" << m_ctrl_flag << "c:" << key_count << "m:" << mod;
				switch(mod){
					case Qt::ControlModifier:
						selectedItem->rotate(1);
					break;
					case Qt::AltModifier:
						selectedItem->scale(1.1,1);
					break;
					case Qt::MetaModifier:
						selectedItem->shear(0.1,0);
					break;
					default:
						selectedItem->translate(1,0);
				}
				m_ctrl_flag = false;
			break;
			case Qt::Key_Left:
				switch(mod){
					case Qt::ControlModifier:
						selectedItem->rotate(-1);
						break;
					case Qt::AltModifier:{
						qreal rec_width = rec.width();
						qDebug() << "x: " << rec_width;
						rec_width -= rec_width*1.1;
						qDebug() << "a andar: " << -rec_width;
						selectedItem->translate(rec_width,0);
						selectedItem->scale(1.1,1);
						qDebug() << "new size: " << (selectedItem->sceneBoundingRect()).width();
						break;
					}
					case Qt::MetaModifier:
						selectedItem->shear(-0.1,0);
						break;
					default:
						selectedItem->translate(-1,0);
				}
			break;
			case Qt::Key_Up:
				switch(mod){
					case Qt::AltModifier:{
						qreal rec_height= rec.height();
						rec_height-= rec_height*1.1;
						selectedItem->translate(0,rec_height);
						selectedItem->scale(1,1.1);
						break;
					}
					case Qt::MetaModifier:
						selectedItem->shear(0,0.1);
						break;
					default:
						selectedItem->translate(0,-1);
				}
				
			break;
			case Qt::Key_Down:
				switch(mod){
					case Qt::AltModifier: 
						selectedItem->scale(1,1.1);
						break;
					case Qt::MetaModifier:
						selectedItem->shear(0,-0.1);
						break;
					default:
						selectedItem->translate(0,1);
				}
			break;
			case Qt::Key_Delete:
				qDebug() << "remove!";
				scene.removeItem(selectedItem);
			break;
		 }
  	}
}

void DragWidgetGrid::copyToClipboard()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setPixmap(QPixmap::grabWidget(this,NULL));
}

void DragWidgetGrid::saveToFile()
{
    QString name = QFileDialog::getSaveFileName(this, QLatin1String("Save as image"), QString(), QLatin1String("*.png"));
    if (!name.endsWith(QLatin1String(".png")))
        name.append(QLatin1String(".png"));
   QPixmap::grabWidget(this,NULL).save(name, "PNG");
}

void DragWidgetGrid::sendToPrinter(){
	
	QPrinter printer;

     QPrintDialog *dialog = new QPrintDialog(&printer, this);
     dialog->setWindowTitle(tr("Print Document"));
     if (dialog->exec() != QDialog::Accepted)
         return;
	QPainter painter;
	painter.begin(&printer);
	painter.drawPixmap(0,0,QPixmap::grabWidget(this,NULL));
	painter.end();
}


/************************************ writeStream ************************************/

void  DragWidgetGrid::resetView(){
 foreach( QGraphicsItem*  item, items() )
  {
	scene.removeItem(item);  
  }
  resetMatrix();
  resetTransform();
}

void  DragWidgetGrid::SaveProject( QXmlStreamWriter* stream )
{
  // write station data to xml stream
  foreach( QGraphicsItem*  item, items() )
  {
    QGraphicsLineItem *  item_line = dynamic_cast<QGraphicsLineItem*>( item );
    if ( item_line )
    {
      stream->writeEmptyElement( "line" );
	  QLineF line = item_line->line();
      stream->writeAttribute( "x1", QString("%1").arg(line.x1()));
      stream->writeAttribute( "y1", QString("%1").arg(line.y1()));
	  stream->writeAttribute( "x2", QString("%1").arg(line.x2()));
      stream->writeAttribute( "y2", QString("%1").arg(line.y2()));
    }
	
	 QGraphicsPixmapItem *  pixmap = dynamic_cast<QGraphicsPixmapItem*>( item );
	 if ( pixmap )
    {
	  QMatrix m = pixmap->sceneMatrix ();
      stream->writeEmptyElement( "pixmap" );
      stream->writeAttribute( "id", QString("%1").arg(pixmap->data(ObjectID).toString()));
	  stream->writeAttribute( "m11", QString("%1").arg(m.m11 ()  ));
	  stream->writeAttribute( "m12", QString("%1").arg(m.m12 () ));
	  stream->writeAttribute( "m21", QString("%1").arg(m.m21 () ));
	  stream->writeAttribute( "m22", QString("%1").arg(m.m22 () ));  
	  stream->writeAttribute( "dx", QString("%1").arg(m.dx ()  ));
	  stream->writeAttribute( "dy", QString("%1").arg(m.dy ()));
    }
	
  }
}

void  DragWidgetGrid::LoadProject( QXmlStreamReader* stream )
{
  // read station data from xml stream

  while ( !stream->atEnd() )
  {
    stream->readNext();
    if ( stream->isStartElement() && stream->name() == "line" )
    {
      qreal x1 = 0.0, y1 = 0.0,x2 = 0.0,y2=0.0;
      foreach( QXmlStreamAttribute attribute, stream->attributes() )
      {
        if ( attribute.name() == "x1" ) x1 = attribute.value().toString().toDouble();
        if ( attribute.name() == "y1" ) y1 = attribute.value().toString().toDouble();
		if ( attribute.name() == "x1" ) x2 = attribute.value().toString().toDouble();
        if ( attribute.name() == "y1" ) y2 = attribute.value().toString().toDouble();
      }
      scene.addLine(QLineF(x1,y1,x2,y2),QPen(Qt::SolidLine));
    }
	
	if ( stream->isStartElement() && stream->name() == "pixmap" )
    {
      qreal m11 = 0.0;
	  qreal m12 = 0.0;
	  qreal m21 = 0.0;
	  qreal m22 = 0.0;
	  qreal dx = 0.0;
	  qreal dy = 0.0;
	  QString id;
      foreach( QXmlStreamAttribute attribute, stream->attributes() )
      {
		if ( attribute.name() == "id" ) id = attribute.value().toString();
        if ( attribute.name() == "m11" ) m11 = attribute.value().toString().toDouble();
        if ( attribute.name() == "m12" ) m12 = attribute.value().toString().toDouble();
		if ( attribute.name() == "m21" ) m21 = attribute.value().toString().toDouble();
        if ( attribute.name() == "m22" ) m22 = attribute.value().toString().toDouble();
		if ( attribute.name() == "dx" ) dx = attribute.value().toString().toDouble();
        if ( attribute.name() == "dy" ) dy = attribute.value().toString().toDouble();
      }
	  qDebug() << id;
	  QMatrix m = QMatrix(m11,m12,m21,m22,dx,dy);
	  // we need to add the pixmap here
	  QPixmap p = svg_list->getPixmapByName(id);

	  QGraphicsPixmapItem * pixmap = scene.addPixmap(p);
	  
	  pixmap->setMatrix(m,false);
	}	
	
  }
	
}
