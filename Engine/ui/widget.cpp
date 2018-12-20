#include "widget.h"

#include <Tempest/Layout>

using namespace Tempest;

struct Widget::Iterator {
  Iterator(Widget* owner):nodes(&owner->wx){
    //owner->iterator=this;
    }

  size_t                id=0;
  std::vector<Widget*>* nodes;
  std::vector<Widget*>  deleteLater;
  };

Widget::Widget() {
  lay = new(layBuf) Layout();
  lay->bind(this);
  }

Widget::~Widget() {
  removeAllWidgets();
  freeLayout();
  }

void Widget::setLayout(Orienration ori) noexcept {
  static_assert(sizeof(LinearLayout)<=sizeof(layBuf),"layBuf is too small");
  freeLayout();
  new(layBuf) LinearLayout(ori);
  lay=reinterpret_cast<Layout*>(layBuf);
  lay->bind(this);
  }

void Widget::setLayout(Layout *l) {
  if(l==nullptr)
    throw std::invalid_argument("null layout");
  freeLayout();
  lay = l;
  lay->bind(this);
  }

void Widget::applyLayout() {
  lay->applyLayout();
  }

void Widget::removeAllWidgets() {
  std::vector<Widget*> rm=std::move(wx);
  for(auto& w:rm)
    w->ow=nullptr;
  for(auto& w:rm)
    delete w;
  }

void Widget::freeLayout() noexcept {
  if(reinterpret_cast<char*>(lay)!=layBuf){
    delete lay;
    } else {
    layout().~Layout();
    }
  }

void Widget::implDisableSum(Widget *root,int diff) noexcept {
  root->state.disable += diff;

  const std::vector<Widget*> & w = root->wx;

  for( Widget* wx:w )
    implDisableSum(wx,diff);
  }

void Widget::dispatchPaintEvent(PaintEvent& e) {
  paintEvent(e);
  const size_t count=widgetsCount();
  for(size_t i=0;i<count;++i) {
    Widget& wx=widget(i);

    PaintEvent ex(e,wx.x(),wx.y(),wx.w(),wx.h());
    wx.dispatchPaintEvent(ex);
    }
  }

void Widget::setOwner(Widget *w) {
  if(ow==w)
    return;
  if(ow)
    ow->takeWidget(this);
  ow=w;
  }

Widget& Widget::implAddWidget(Widget *w) {
  if(w==nullptr)
    throw std::invalid_argument("null widget");
  wx.emplace_back(w);
  w->setOwner(this);
  if(state.disable>0)
    implDisableSum(w,state.disable);
  lay->applyLayout();
  return *w;
  }

Widget *Widget::takeWidget(Widget *w) {
  if(state.mouseFocus==w)
    state.mouseFocus=nullptr;
  if(state.moveFocus==w)
    state.moveFocus=nullptr;
  if(state.disable>0)
    implDisableSum(w,-state.disable);
  return lay->takeWidget(w);
  }

Point Widget::mapToRoot(const Point &p) const {
  const Widget* w=this;
  Point ret=p;

  while(w!=nullptr){
    ret += w->pos();
    w   =  w->owner();
    }

  return ret;
  }

void Widget::setPosition(int x, int y) {
  if(wrect.x==x && wrect.y==y)
    return;

  wrect.x = x;
  wrect.y = y;
  update();
  }

void Widget::setPosition(const Point &pos) {
  setPosition(pos.x,pos.y);
  }

void Widget::setGeometry(const Rect &rect) {
  if(wrect==rect)
    return;
  bool resize=(wrect.w!=rect.w || wrect.h!=rect.h);
  wrect=rect;
  update();

  if(resize) {
    lay->applyLayout();
    SizeEvent e(uint32_t(rect.w),uint32_t(rect.h));
    resizeEvent( e );
    }
  }

void Widget::setGeometry(int x, int y, int w, int h) {
  setGeometry(Rect(x,y,w,h));
  }

void Widget::resize(const Size &size) {
  resize(size.w,size.h);
  }

void Widget::resize(int w, int h) {
  if(wrect.w==w && wrect.h==h)
    return;
  wrect.w=w;
  wrect.h=h;

  lay->applyLayout();
  SizeEvent e(w,h);
  resizeEvent( e );
  }

void Widget::setSizeHint(const Size &s) {
  if(szHint==s)
    return;
  szHint=s;
  if(ow!=nullptr)
    ow->lay->applyLayout();
  }

void Widget::setSizeHint(const Size &s, const Margin &add) {
  setSizeHint(Size(s.w+add.xMargin(),s.h+add.yMargin()));
  }

void Widget::setSizePolicy(SizePolicyType hv) {
  setSizePolicy(hv,hv);
  }

void Widget::setSizePolicy(SizePolicyType h, SizePolicyType v) {
  if(szPolicy.typeH==h && szPolicy.typeV==v)
    return;
  szPolicy.typeH=h;
  szPolicy.typeV=v;

  if(ow!=nullptr)
    ow->lay->applyLayout();
  }

void Widget::setSizePolicy(const SizePolicy &sp) {
  if(szPolicy==sp)
    return;
  szPolicy=sp;
  if(ow!=nullptr)
    ow->lay->applyLayout();
  }

void Widget::setFocusPolicy(FocusPolicy f) {
  fcPolicy=f;
  }

void Widget::setMargins(const Margin &m) {
  if(marg!=m){
    marg=m;
    lay->applyLayout();
    }
  }

void Widget::setSpacing(int s) {
  if(s!=spa){
    spa=s;
    lay->applyLayout();
    }
  }

void Widget::setMaximumSize(const Size &s) {
  if(szPolicy.maxSize==s)
    return;

  szPolicy.maxSize = s;

  if( owner() )
    owner()->applyLayout();

  if(wrect.w>s.w || wrect.h>s.h)
    setGeometry( wrect.x, wrect.y, std::min(s.w, wrect.w), std::min(s.h, wrect.h) );
  }

void Widget::setMinimumSize(const Size &s) {
  if(szPolicy.minSize==s)
    return;

  szPolicy.minSize = s;
  if( owner() )
    owner()->applyLayout();

  if(wrect.w<s.w || wrect.h<s.h )
    setGeometry( wrect.x, wrect.y, std::max(s.w, wrect.w), std::max(s.h, wrect.h) );
  }

void Widget::setMaximumSize(int w, int h) {
  setMaximumSize( Size(w,h) );
  }

void Widget::setMinimumSize(int w, int h) {
  setMinimumSize( Size(w,h) );
  }

Rect Widget::clentRet() const {
  return Rect(marg.left,marg.right,wrect.w-marg.xMargin(),wrect.h-marg.yMargin());
  }

void Widget::setEnabled(bool e) {
  if(e!=wstate.disabled)
    return;

  if(!wstate.disabled) {
    wstate.disabled=true;
    implDisableSum(this,+1);
    } else {
    wstate.disabled=false;
    implDisableSum(this,-1);
    }
  update();
  }

bool Widget::isEnabled() const {
  return state.disable==0;
  }

bool Widget::isEnabledTo(const Widget *ancestor) const {
  const Widget* w = this;
  while( w ){
    if(w->wstate.disabled)
       return false;
    if( w==ancestor )
       return true;
    w = w->owner();
    }
  return true;
  }

void Widget::setFocus(bool b) {
  implSetFocus(&Additive::clickFocus,&State::focus,b);
  }

void Widget::implSetFocus(Widget* Additive::*add, bool State::*flag, bool value) {
  if(wstate.*flag==value)
    return;

  Widget* w=this;
  while(true) {
    if(w->state.*add!=nullptr || w->owner()==nullptr) {
      while(true) {
        auto mv=w->state.*add;
        w->state.*add=nullptr;
        if(mv==nullptr){
          w->wstate.*flag=false;
          break;
          }
        w=mv;
        }

      w=this;
      wstate.*flag=true;
      while(w->owner()) {
        w->owner()->state.*add=w;
        w = w->owner();
        }
      break;
      }
    w = w->owner();
    }
  }

void Widget::update() {
  Widget* w=this;
  while(w!=nullptr){
    if(w->state.needToUpdate)
      return;
    w->state.needToUpdate=true;
    w = w->owner();
    }
  }

void Widget::paintEvent(PaintEvent&) {
  }

void Widget::resizeEvent(SizeEvent &e) {
  e.accept();
  }

void Widget::mouseDownEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseUpEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseMoveEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseDragEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::dispatchMoveEvent(MouseEvent &event) {
  switch(event.type()){
    case Event::MouseDown: dispatchMouseDown(event); break;
    case Event::MouseUp:   dispatchMouseUp  (event); break;
    case Event::MouseMove: dispatchMouseMove(event); break;
    case Event::MouseDrag: dispatchMouseDrag(event); break;
    default:break;
    }
  }

void Widget::dispatchMouseDown(MouseEvent &event) {
  Point pos=event.pos();
  for(auto i:wx){
    if(i->rect().contains(pos)){
      MouseEvent ex(event.x - i->x(),
                    event.y - i->y(),
                    event.button,
                    event.delta,
                    event.mouseID,
                    event.type());
      i->dispatchMoveEvent(ex);
      if(ex.isAccepted()) {
        state.mouseFocus=i;
        event.accept();
        return;
        }
      }
    }

  mouseDownEvent(event);
  }

void Widget::dispatchMouseUp(MouseEvent &event) {
  if(state.mouseFocus!=nullptr) {
    MouseEvent ex(event.x - state.mouseFocus->x(),
                  event.y - state.mouseFocus->y(),
                  event.button,
                  event.delta,
                  event.mouseID,
                  event.type());
    state.mouseFocus->dispatchMouseUp(ex);
    state.mouseFocus=nullptr;
    } else {
    mouseUpEvent(event);
    }
  }

void Widget::dispatchMouseMove(MouseEvent &event) {
  Point pos=event.pos();
  for(auto i:wx){
    if(i->rect().contains(pos)){
      MouseEvent ex(event.x - i->x(),
                    event.y - i->y(),
                    event.button,
                    event.delta,
                    event.mouseID,
                    event.type());
      i->dispatchMoveEvent(ex);
      if(ex.isAccepted()) {
        implSetFocus(&Additive::moveFocus,&State::moveFocus,true);
        event.accept();
        return;
        }
      }
    }

  mouseMoveEvent(event);
  }

void Widget::dispatchMouseDrag(MouseEvent &event) {
  if(state.mouseFocus!=nullptr) {
    MouseEvent ex(event.x - state.mouseFocus->x(),
                  event.y - state.mouseFocus->y(),
                  event.button,
                  event.delta,
                  event.mouseID,
                  event.type());
    state.mouseFocus->dispatchMouseDrag(ex);
    } else {
    mouseDragEvent(event);
    }
  }