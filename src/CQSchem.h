#ifndef CQSchem_H
#define CQSchem_H

#include <CDisplayRange2D.h>
#include <CDisplayTransform2D.h>
#include <QFrame>
#include <QPainter>

class CQSchem;
class CQGate;
class CQPort;
class CQConnection;
class CQBus;
class CQPlacementGroup;

class QLabel;
class QPainter;

struct CQSchemRenderer {
  CQSchem*            schem { nullptr };
  QPainter*           painter { nullptr };
  QRect               prect;
  QRectF              rect;
  CDisplayRange2D     displayRange;
  CDisplayTransform2D displayTransform;

  CQSchemRenderer() :
   displayTransform(&displayRange) {
  }

  // to pixel
  QPointF windowToPixel(const QPointF &w) const {
    double wx1, wy1;

    displayTransform.getMatrix().multiplyPoint(w.x(), w.y(), &wx1, &wy1);

    double px, py;

    displayRange.windowToPixel(wx1, wy1, &px, &py);

    return QPointF(px, py);
  }

  double windowWidthToPixelWidth(double w) const {
    return std::abs(windowToPixel(QPointF(w, 0)).x() - windowToPixel(QPointF(0, 0)).x());
  }

  double windowHeightToPixelHeight(double h) const {
    return std::abs(windowToPixel(QPointF(0, h)).y() - windowToPixel(QPointF(0, 0)).y());
  }

  QRectF windowToPixel(const QRectF &r) const {
    QPointF p1 = windowToPixel(r.topLeft    ());
    QPointF p2 = windowToPixel(r.bottomRight());

    return QRectF(p1, p2);
  }

  // from pixel
  QPointF pixelToWindow(const QPointF &p) const {
    double wx, wy;

    displayRange.pixelToWindow(p.x(), p.y(), &wx, &wy);

    double wx1, wy1;

    displayTransform.getIMatrix().multiplyPoint(wx, wy, &wx1, &wy1);

    return QPointF(wx1, wy1);
  }

  double pixelWidthToWindowWidth(double w) const {
    return std::abs(pixelToWindow(QPointF(w, 0)).x() - pixelToWindow(QPointF(0, 0)).x());
  }

  double pixelHeightToWindowHeight(double h) const {
    return std::abs(pixelToWindow(QPointF(0, h)).y() - pixelToWindow(QPointF(0, 0)).y());
  }

  void setFontSize(double h) {
    QFont font = painter->font();

    double scale = 1;

    for (int i = 0; i < 8; ++i) {
      font.setPointSizeF(h*scale);

      QFontMetricsF fm(font);

      double h1 = fm.height();

      scale *= h/h1;
    }

    if (font.pointSizeF() > 24)
      font.setPointSizeF(24);

    painter->setFont(font);
  }
};

//---

class CQPlacementGroup {
 public:
  enum class Placement {
    VERTICAL,
    HORIZONTAL,
    GRID
  };

  enum class Alignment {
    LEFT,
    CENTER,
    RIGHT,
    HFILL,
    VFILL,
    FILL
  };

  struct GateData {
    CQGate*   gate      { nullptr };
    int       r         { -1 };
    int       c         { -1 };
    int       nr        { 1 };
    int       nc        { 1 };
    Alignment alignment { Alignment::CENTER };

    GateData(CQGate *gate, int r=-1, int c=-1, int nr=1, int nc=1,
             Alignment alignment=Alignment::CENTER) :
     gate(gate), r(r), c(c), nr(nr), nc(nc), alignment(alignment) {
    }
  };

  struct PlacementGroupData {
    CQPlacementGroup *placementGroup { nullptr };
    int               r              { -1 };
    int               c              { -1 };
    int               nr             { 1 };
    int               nc             { 1 };
    Alignment         alignment      { Alignment::CENTER };

    PlacementGroupData(CQPlacementGroup *placementGroup, int r=-1, int c=-1,
                       int nr=1, int nc=1, Alignment alignment=Alignment::CENTER) :
     placementGroup(placementGroup), r(r), c(c), nr(nr), nc(nc), alignment(alignment) {
    }
  };

  using Gates           = std::vector<GateData>;
  using PlacementGroups = std::vector<PlacementGroupData>;

 public:
  CQPlacementGroup(const Placement &placement=Placement::HORIZONTAL, int nr=-1, int nc=-1);

  const Placement &placement() const { return placement_; }
  void setPlacement(const Placement &v) { placement_ = v; }

  int numRows() const { return nr_; }
  void setNumRows(int i) { nr_ = i; }

  int numColumns() const { return nc_; }
  void setNumColumns(int i) { nc_ = i; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const Gates &gates() const { return gates_; }

  const PlacementGroups &placementGroups() const { return placementGroups_; }

  const QRectF &rect() const { updateRect(); return rect_; }
  void setRect(const QRectF &r);

  void addGate(CQGate *gate, int r=-1, int c=-1, int nr=1, int nc=1,
               Alignment alignment=Alignment::CENTER);

  void removeGate(CQGate *gate);

  CQPlacementGroup *addPlacementGroup(const Placement &placement, int nr, int nc,
                                      int r1=-1, int c1=-1, int nr1=1, int nc1=1,
                                      Alignment alignment=Alignment::CENTER);

  void addPlacementGroup(CQPlacementGroup *placementGroup, int r=-1, int c=-1,
                         int nr=1, int nc=1, Alignment alignment=Alignment::CENTER);

  bool inside(const QPointF &p) const { return prect_.contains(p); }

  QSizeF calcSize() const;

  void place();

  void draw(CQSchemRenderer *renderer) const;

 private:
  void updateRect() const;

  QColor penColor(CQSchemRenderer *renderer) const;

 private:
  Placement         placement_           { Placement::HORIZONTAL };
  int               nr_                  { -1 };
  int               nc_                  { -1 };
  Gates             gates_;
  CQPlacementGroup* parentPlacementGroup { nullptr };
  PlacementGroups   placementGroups_;
  QRectF            rect_;
  bool              rectValid_           { false };
  mutable QRectF    prect_;
  double            w_                   { 1.0 };
  double            h_                   { 1.0 };
  bool              selected_            { false };
};

//---

class CQSchemWindow : public QFrame {
  Q_OBJECT

 public:
  CQSchemWindow();

  CQSchem *schem() const { return schem_; }

  void setPos(const QPointF &pos);

  QSize sizeHint() const override;

 private slots:
  void connectionTextSlot(bool b);
  void gateTextSlot      (bool b);
  void portTextSlot      (bool b);

  void moveGateSlot      (bool b);
  void movePlacementSlot (bool b);
  void moveConnectionSlot(bool b);

  void connectionVisibleSlot    (bool b);
  void gateVisibleSlot          (bool b);
  void placementGroupVisibleSlot(bool b);

 private:
  CQSchem* schem_    { nullptr };
  QLabel*  posLabel_ { nullptr };
};

//---

class CQSchem : public QFrame {
  Q_OBJECT

 public:
  using Gates = std::vector<CQGate *>;

 public:
  CQSchem(CQSchemWindow *window);
 ~CQSchem();

  bool isShowConnectionText() const { return showConnectionText_; }
  void setShowConnectionText(bool b) { showConnectionText_ = b; update(); }

  bool isShowGateText() const { return showGateText_; }
  void setShowGateText(bool b) { showGateText_ = b; update(); }

  bool isShowPortText() const { return showPortText_; }
  void setShowPortText(bool b) { showPortText_ = b; update(); }

  bool isMoveGate() const { return moveGate_; }
  void setMoveGate(bool b) { moveGate_ = b; }

  bool isMovePlacement() const { return movePlacement_; }
  void setMovePlacement(bool b) { movePlacement_ = b; }

  bool isMoveConnection() const { return moveConnection_; }
  void setMoveConnection(bool b) { moveConnection_ = b; }

  bool isConnectionVisible() const { return connectionVisible_; }
  void setConnectionVisible(bool b) { connectionVisible_ = b; }

  bool isGateVisible() const { return gateVisible_; }
  void setGateVisible(bool b) { gateVisible_ = b; }

  bool isPlacementGroupVisible() const { return placementGroupVisible_; }
  void setPlacementGroupVisible(bool b) { placementGroupVisible_ = b; }

  void addNandGate      ();
  void addNotGate       ();
  void addAndGate       ();
  void addAnd3Gate      ();
  void addAnd4Gate      ();
  void addAnd8Gate      ();
  void addOrGate        ();
  void addXorGate       ();
  void addMemoryGate    ();
  void addMemory8Gate   ();
  void addEnablerGate   ();
  void addRegisterGate  ();
  void addDecoder4Gate  ();
  void addDecoder8Gate  ();
  void addDecoder16Gate ();
  void addDecoder256Gate();
  void addLShiftGate    ();
  void addRShiftGate    ();
  void addAdderGate     ();

  void buildNotGate       ();
  void buildAndGate       ();
  void buildAnd3Gate      ();
  void buildAnd4Gate      ();
  void buildAnd8Gate      ();
  void buildOrGate        ();
  void buildXorGate       ();
  void buildMemoryGate    ();
  void buildMemory8Gate   ();
  void buildEnablerGate   ();
  void buildRegisterGate  ();
  void buildDecoder4Gate  ();
  void buildDecoder8Gate  ();
  void buildDecoder16Gate ();
  void buildDecoder256Gate();
  void buildLShift        ();
  void buildRShift        ();
  void buildInverter      ();
  void buildAnder         ();
  void buildOrer          ();
  void buildXorer         ();
  void buildAdder         ();
  void buildAdder8        ();
  void buildComparator    ();
  void buildComparator8   ();
  void buildRam256        ();
  void buildRam65536      ();

  CQConnection *addConnection(const QString &name);

  void addGate(CQGate *gate);

  template<typename T>
  T *addGateT(const QString &name="") {
    T *gate = new T(name);

    addGate(gate);

    return gate;
  }

  CQBus *addBus(const QString &name, int n);

  CQPlacementGroup *addPlacementGroup(CQPlacementGroup::Placement placement=
                                       CQPlacementGroup::Placement::HORIZONTAL,
                                      int nr=-1, int nc=-1);

  void addPlacementGroup(CQPlacementGroup *placementGroup);

  void place();

  void calcBounds();

  void exec();

  void resizeEvent(QResizeEvent *);

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

  CQGate*           nearestGate          (const QPointF &p) const;
  CQPlacementGroup* nearestPlacementGroup(const QPointF &p) const;
  CQConnection*     nearestConnection    (const QPointF &p) const;

  CQGate*           insideGate      () const { return insideGate_      ; }
  CQPlacementGroup* insidePlacement () const { return insidePlacement_ ; }
  CQConnection*     insideConnection() const { return insideConnection_; }

  void selectedGates(Gates &gates) const;

  static void drawConnection(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2);

  static void drawTextInRect(CQSchemRenderer *renderer, const QRectF &r, const QString &text);
  static void drawTextAtPoint(CQSchemRenderer *renderer, const QPointF &p, const QString &text);

 private:
  using Buses           = std::vector<CQBus *>;
  using PlacementGroups = std::vector<CQPlacementGroup *>;
  using Connections     = std::vector<CQConnection *>;

  CQSchemWindow*    window_                { nullptr };
  bool              showConnectionText_    { true };
  bool              showGateText_          { true };
  bool              showPortText_          { false };
  bool              moveGate_              { true };
  bool              movePlacement_         { false };
  bool              moveConnection_        { false };
  bool              connectionVisible_     { true };
  bool              placementGroupVisible_ { false };
  bool              gateVisible_           { true };
  Gates             gates_;
  Buses             buses_;
  Connections       connections_;
  CQPlacementGroup* placementGroup_        { nullptr };
  QRectF            rect_;
  CQSchemRenderer   renderer_;
  QPointF           pressPoint_;
  bool              pressed_               { false };
  CQGate*           pressGate_             { nullptr };
  CQPlacementGroup* pressPlacement_        { nullptr };
  CQConnection*     pressConnection_       { nullptr };
  CQGate*           insideGate_            { nullptr };
  CQPlacementGroup* insidePlacement_       { nullptr };
  CQConnection*     insideConnection_      { nullptr };
  QPointF           movePoint_;
};

//---

class CQConnection {
 public:
  struct Line {
    int     ind;
    QPointF start;
    QPointF end;

    Line(int ind, const QPointF &start, const QPointF &end) :
     ind(ind), start(start), end(end) {
    }
  };

  using Ports = std::vector<CQPort *>;
  using Lines = std::vector<Line>;

 public:
  CQConnection(const QString &name="");

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  bool getValue() const { return value_; }
  void setValue(bool b);

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const Ports &inPorts() const { return inPorts_; }
  void addInPort(CQPort *p) { inPorts_.push_back(p); }

  const Ports &outPorts() const { return outPorts_; }
  void addOutPort(CQPort *p) { outPorts_.push_back(p); }

  CQBus *bus() const { return bus_; }
  void setBus(CQBus *bus) { bus_ = bus; }

  const QRectF &prect() const { return prect_; }
  void setPRect(const QRectF &r) { prect_ = r; }

  bool isInput () const { return (inPorts_.empty() && ! outPorts_.empty()); }
  bool isOutput() const { return (! inPorts_.empty() && outPorts_.empty()); }

  bool inside(const QPointF &p) const;

  void draw(CQSchemRenderer *renderer) const;

  QPointF midPoint() const;

  QColor penColor(CQSchemRenderer *renderer) const;

 private:
  void addConnectLines(const QPointF &p1, const QPointF &p2,
                       const QPointF &p3, const QPointF &p4) const;

  void connectPoints(const QPointF &p1, const QPointF &p2) const;

  void addLine(const QPointF &p1, const QPointF &p2) const;

  void drawLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2,
                bool showText) const;

 private:
  QString        name_;
  bool           value_    { false };
  bool           selected_ { false };
  Ports          inPorts_;
  Ports          outPorts_;
  CQBus*         bus_      { nullptr };
  mutable QRectF prect_;
  mutable Lines  lines_;
};

//---

class CQBus {
 public:
  enum class Position {
    START,
    MIDDLE,
    END
  };

 public:
  CQBus(const QString &name="", int n=8);

  const Position &position() const { return position_; }
  void setPosition(const Position &v) { position_ = v; }

  void addConnection(CQConnection *connection, int i);

  int connectionIndex(CQConnection *connection);

  void draw(CQSchemRenderer *renderer);

 private:
  using Connections = std::vector<CQConnection *>;

  QString     name_;
  int         n_            { 8 };
  Connections connections_;
  Position    position_     { Position::MIDDLE };
};

//---

class CQPort {
 public:
  enum class Direction {
    IN,
    OUT
  };

  enum class Side {
    NONE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
  };

 public:
  CQPort(const QString &name, const Direction &direction) :
   name_(name), direction_(direction) {
  }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  const Direction &direction() const { return direction_; }
  void setDirection(const Direction &direction) { direction_ = direction; }

  const Side &side() const { return side_; }
  void setSide(const Side &v) { side_ = v; }

  const CQGate *gate() const { return gate_; }
  void setGate(CQGate *p) { gate_ = p; }

  bool getValue() const { return value_; }
  void setValue(bool b, bool propagate=true);

  CQConnection *connection() const { return connection_; }
  void setConnection(CQConnection *connection) { connection_ = connection; }

  const QPointF &pixelPos() const { return ppos_; }
  void setPixelPos(const QPointF &v) { ppos_ = v; }

  QPointF offsetPixelPos() const;

  static Side nextSide(const Side &side) {
    switch (side) {
      case Side::LEFT  : return Side::TOP;
      case Side::TOP   : return Side::RIGHT;
      case Side::RIGHT : return Side::BOTTOM;
      case Side::BOTTOM: return Side::LEFT;
      default:           return Side::NONE;
    }
  }

  static Side prevSide(const Side &side) {
    switch (side) {
      case Side::LEFT  : return Side::BOTTOM;
      case Side::TOP   : return Side::LEFT;
      case Side::RIGHT : return Side::TOP;
      case Side::BOTTOM: return Side::RIGHT;
      default:           return Side::NONE;
    }
  }

 private:
  QString         name_;
  Direction       direction_  { Direction::IN };
  Side            side_       { Side::NONE };
  CQGate*         gate_       { nullptr };
  bool            value_      { false };
  CQConnection*   connection_ { nullptr };
  mutable QPointF ppos_;
};

//---

class CQGate {
 public:
  enum class Orientation {
    R0,
    R90,
    R180,
    R270
  };

  using Ports = std::vector<CQPort *>;

 public:
  CQGate(const QString &name) :
   name_(name) {
  }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  const Orientation &orientation() const { return orientation_; }
  void setOrientation(const Orientation &v) { orientation_ = v; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  void connect(const QString &name, CQConnection *connection);

  const Ports &inputs() const { return inputs_; }
  const Ports &outputs() const { return outputs_; }

  const QRectF &rect() const { return rect_; }
  void setRect(const QRectF &v) { rect_ = v; }

  CQPlacementGroup *placementGroup() const { return placementGroup_; }
  void setPlacementGroup(CQPlacementGroup *g) { placementGroup_ = g; }

  void addInputPorts(const QStringList &names) {
    for (const auto &name : names)
      addInputPort(name);
  }

  void addOutputPorts(const QStringList &names) {
    for (const auto &name : names)
      addOutputPort(name);
  }

  void addInputPort (const QString &name);
  void addOutputPort(const QString &name);

  CQPort *getPortByName(const QString &name) const;

  double px1() const { return prect_.left  (); }
  double py1() const { return prect_.top   (); }
  double px2() const { return prect_.right (); }
  double py2() const { return prect_.bottom(); }

  double pxm() const { return prect_.center().x(); }
  double pym() const { return prect_.center().y(); }

  //---

  // gaps (top to bottom) in pixels
  std::vector<double> calcXGaps(int n, bool flip=false) const {
    return calcXGaps(px1(), px2(), n, flip);
  }

  std::vector<double> calcXGaps(double px1, double px2, int n, bool flip=false) const {
    assert(n > 0);

    std::vector<double> x(n);

    double xgap = this->xgap(px1, px2, n);

    if (! flip) {
      x[0] = px1 + 0.5*xgap;

      for (int i = 1; i < n; ++i)
        x[i] = x[i - 1] + xgap;
    }
    else {
      x[0] = px2 - 0.5*xgap;

      for (int i = 1; i < n; ++i)
        x[i] = x[i - 1] - xgap;
    }

    return x;
  }

  double xgap(int n) const { return xgap(px1(), px2(), n); }

  double xgap(double px1, double px2, int n) const { return (px2 - px1)/n; }

  //---

  // gaps (top to bottom) in pixels
  std::vector<double> calcYGaps(int n, bool flip=false) const {
    return calcYGaps(py1(), py2(), n, flip);
  }

  std::vector<double> calcYGaps(double py1, double py2, int n, bool flip=false) const {
    assert(n > 0);

    std::vector<double> y(n);

    double ygap = this->ygap(py1, py2, n);

    if (! flip) {
      y[0] = py1 + 0.5*ygap;

      for (int i = 1; i < n; ++i)
        y[i] = y[i - 1] + ygap;
    }
    else {
      y[0] = py2 - 0.5*ygap;

      for (int i = 1; i < n; ++i)
        y[i] = y[i - 1] - ygap;
    }

    return y;
  }

  double ygap(int n) const { return ygap(py1(), py2(), n); }

  double ygap(double py1, double py2, int n) const { return (py2 - py1)/n; }

  //---

  double width() const {
    return (orientation() == Orientation::R0 || orientation() == Orientation::R180 ? w_ : h_);
  }

  double height() const {
    return (orientation() == Orientation::R0 || orientation() == Orientation::R180 ? h_ : w_);
  }

  void setWidth (double w) {
    orientation() == Orientation::R0 || orientation() == Orientation::R180 ? w_ = w : h_ = w; }
  void setHeight(double h) {
    orientation() == Orientation::R0 || orientation() == Orientation::R180 ? h_ = h : w_ = h; }

  double xmargin() const { return width ()*margin_; }
  double ymargin() const { return height()*margin_; }

  //---

  bool inside(const QPointF &p) const { return prect_.contains(p); }

  virtual QSizeF calcSize() const;

  virtual bool exec() = 0;

  virtual void draw(CQSchemRenderer *renderer) const;

  void drawAnd(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const;

  void drawOr(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2,
              double &x3, double &y3) const;

  void drawXor(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const;

  void drawNot(CQSchemRenderer *renderer) const;

  void drawNotIndicator(CQSchemRenderer *renderer) const;

  void placePorts(int ni=-1, int no=-1) const;
  void placePorts(double pix1, double piy1, double pix2, double piy2,
                  double pox1, double poy1, double pox2, double poy2, int ni=-1, int no=-1) const;

  void placePortOnSide(CQPort *port, const CQPort::Side &side) const;

  void placePortsOnSide(CQPort **ports, int n, const CQPort::Side &side) const;

  QColor penColor(CQSchemRenderer *renderer) const;

  void initRect(CQSchemRenderer *renderer) const;

  void nextOrient() { orientation_ = nextOrientation(orientation_); rotateRect(); }
  void prevOrient() { orientation_ = prevOrientation(orientation_); rotateRect(); }

  static Orientation nextOrientation(const Orientation &orient) {
    switch (orient) {
      case Orientation::R0  : return Orientation::R90;
      case Orientation::R90 : return Orientation::R180;
      case Orientation::R180: return Orientation::R270;
      case Orientation::R270: return Orientation::R0;
      default               : { assert(false); return Orientation::R0; }
    }
  }

  static Orientation prevOrientation(const Orientation &orient) {
    switch (orient) {
      case Orientation::R0  : return Orientation::R270;
      case Orientation::R90 : return Orientation::R0;
      case Orientation::R180: return Orientation::R90;
      case Orientation::R270: return Orientation::R180;
      default               : { assert(false); return Orientation::R0; }
    }
  }

  void rotateRect() {
    QPointF c = rect_.center();
    double  w = rect_.width();
    double  h = rect_.height();

    std::swap(w, h);

    rect_ = QRectF(c.x() - w/2.0, c.y() - h/2.0, w, h);
  }

 protected:
  QString           name_;
  Orientation       orientation_    { Orientation::R0 };
  bool              selected_       { false };
  Ports             inputs_;
  Ports             outputs_;
  QRectF            rect_;
  mutable QRectF    prect_;
  double            w_              { 1.0 };
  double            h_              { 0.8 };
  double            margin_         { 0.1 };
  CQPlacementGroup* placementGroup_ { nullptr };
};

//---

class CQNandGate : public CQGate {
 public:
  CQNandGate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQNotGate : public CQGate {
 public:
  CQNotGate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQAndGate : public CQGate {
 public:
  CQAndGate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQAnd3Gate : public CQGate {
 public:
  CQAnd3Gate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQAnd4Gate : public CQGate {
 public:
  CQAnd4Gate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQAnd8Gate : public CQGate {
 public:
  CQAnd8Gate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQOrGate : public CQGate {
 public:
  CQOrGate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQXorGate : public CQGate {
 public:
  CQXorGate(const QString &name="");

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

//---

class CQMemoryGate : public CQGate {
 public:
  CQMemoryGate(const QString &name);

  const CQPort::Side &sside() const { return sside_; }
  void setSSide(const CQPort::Side &s) { sside_ = s; }

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

 private:
  bool         state_ { false };
  CQPort::Side sside_ { CQPort::Side::LEFT };
};

//---

class CQMemory8Gate : public CQGate {
 public:
  CQMemory8Gate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[8] {};
};

//---

class CQEnablerGate : public CQGate {
 public:
  CQEnablerGate(const QString &name);

  const CQPort::Side &eside() const { return eside_; }
  void setESide(const CQPort::Side &s) { eside_ = s; }

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  CQPort::Side eside_ { CQPort::Side::LEFT };
};

//---

class CQRegisterGate : public CQGate {
 public:
  CQRegisterGate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[8] {};
};

//---

class CQDecoder4Gate : public CQGate {
 public:
  CQDecoder4Gate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) {
    switch (i) {
      case 0: return "a";
      case 1: return "b";
    }

    assert(false);
    return "";
  }

  static QString oname(int i) {
    int i1 = (i & 1);
    int i2 = (i & 2) >> 1;

    return QString("%1/%2").arg(i2).arg(i1);
  }
};

//---

class CQDecoder8Gate : public CQGate {
 public:
  CQDecoder8Gate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) {
    switch (i) {
      case 0: return "a";
      case 1: return "b";
      case 2: return "c";
    }

    assert(false);
    return "";
  }

  static QString oname(int i) {
    int i1 = (i & 1);
    int i2 = (i & 2) >> 1;
    int i3 = (i & 4) >> 2;
    int i4 = (i & 8) >> 3;

    return QString("%1/%2/%3/%4").arg(i4).arg(i3).arg(i2).arg(i1);
  }
};

//---

class CQDecoder16Gate : public CQGate {
 public:
  CQDecoder16Gate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) {
    switch (i) {
      case 0: return "a";
      case 1: return "b";
      case 2: return "c";
      case 3: return "d";
    }

    assert(false);
    return "";
  }

  static QString oname(int i) {
    int i1 = (i & 1);
    int i2 = (i & 2) >> 1;
    int i3 = (i & 4) >> 2;
    int i4 = (i & 8) >> 2;

    return QString("%1/%2/%3/%4").arg(i4).arg(i3).arg(i2).arg(i1);
  }
};

//---

class CQDecoder256Gate : public CQGate {
 public:
  CQDecoder256Gate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) {
    switch (i) {
      case 0: return "a";
      case 1: return "b";
      case 2: return "c";
      case 3: return "d";
      case 4: return "e";
      case 5: return "f";
      case 6: return "g";
      case 7: return "h";
    }

    assert(false);
    return "";
  }

  static QString oname(int i) {
    int i1 = (i &   1);
    int i2 = (i &   2) >> 1;
    int i3 = (i &   4) >> 2;
    int i4 = (i &   8) >> 3;
    int i5 = (i &  16) >> 4;
    int i6 = (i &  32) >> 5;
    int i7 = (i &  64) >> 6;
    int i8 = (i & 128) >> 7;

    return QString("%1/%2/%3/%4/%5/%6/%7/%8").
            arg(i8).arg(i7).arg(i6).arg(i5).arg(i4).arg(i3).arg(i2).arg(i1);
  }
};

//---

class CQLShiftGate : public CQGate {
 public:
  CQLShiftGate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[9];
};

//---

class CQRShiftGate : public CQGate {
 public:
  CQRShiftGate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[9];
};

//---

class CQAdderGate : public CQGate {
 public:
  CQAdderGate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;
};

#endif
