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
    return windowToPixel(QPointF(w, 0)).x() - windowToPixel(QPointF(0, 0)).x();
  }

  double windowHeightToPixelHeight(double h) const {
    return windowToPixel(QPointF(0, h)).y() - windowToPixel(QPointF(0, 0)).y();
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
    return pixelToWindow(QPointF(w, 0)).x() - pixelToWindow(QPointF(0, 0)).x();
  }

  double pixelHeightToWindowHeight(double h) const {
    return pixelToWindow(QPointF(0, h)).y() - pixelToWindow(QPointF(0, 0)).y();
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

  const QRectF &rect() const { return rect_; }
  void setRect(const QRectF &r);

  void addGate(CQGate *gate, int r=-1, int c=-1, int nr=1, int nc=1,
               Alignment alignment=Alignment::CENTER);

  void removeGate(CQGate *gate);

  void addPlacementGroup(CQPlacementGroup *placementGroup, int r=-1, int c=-1,
                         int nr=1, int nc=1, Alignment alignment=Alignment::CENTER);

  QSizeF calcSize() const;

  void place();

  void draw(CQSchemRenderer *renderer) const;

 private:
  Placement         placement_           { Placement::HORIZONTAL };
  int               nr_                  { -1 };
  int               nc_                  { -1 };
  Gates             gates_;
  CQPlacementGroup* parentPlacementGroup { nullptr };
  PlacementGroups   placementGroups_;
  QRectF            rect_;
  double            w_                   { 1.0 };
  double            h_                   { 1.0 };
};

//---

class CQSchemWindow : public QFrame {
  Q_OBJECT

 public:
  CQSchemWindow();

  CQSchem *schem() const { return schem_; }

  QSize sizeHint() const override;

 private slots:
  void connectionTextSlot(bool b);
  void gateTextSlot(bool b);

 private:
  CQSchem *schem_ { nullptr };
};

//---

class CQSchem : public QFrame {
  Q_OBJECT

 public:
  CQSchem();

  bool isShowConnectionText() const { return showConnectionText_; }
  void setShowConnectionText(bool b) { showConnectionText_ = b; update(); }

  bool isShowGateText() const { return showGateText_; }
  void setShowGateText(bool b) { showGateText_ = b; update(); }

  void addNandGate     ();
  void addNotGate      ();
  void addAndGate      ();
  void addAnd3Gate     ();
  void addAnd4Gate     ();
  void addMemoryGate   ();
  void addMemory8Gate  ();
  void addEnablerGate  ();
  void addRegisterGate ();
  void addDecoder4Gate ();
  void addDecoder8Gate ();
  void addDecoder16Gate();

  void buildNotGate      ();
  void buildAndGate      ();
  void buildAnd3Gate     ();
  void buildAnd4Gate     ();
  void buildMemoryGate   ();
  void buildMemory8Gate  ();
  void buildEnablerGate  ();
  void buildRegisterGate ();
  void buildDecoder4Gate ();
  void buildDecoder8Gate ();
  void buildDecoder16Gate();
  void buildRam256       ();

  CQConnection *addConnection(const QString &name);

  void addGate(CQGate *gate);

  CQPlacementGroup *addPlacementGroup(CQPlacementGroup::Placement placement=
                                       CQPlacementGroup::Placement::HORIZONTAL,
                                      int nr=-1, int nc=-1);

  void addPlacementGroup(CQPlacementGroup *placementGroup);

  template<typename T>
  T *addGateT(const QString &name="") {
    T *gate = new T(name);

    addGate(gate);

    return gate;
  }

  void place();

  void calcBounds();

  void exec();

  void resizeEvent(QResizeEvent *);

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

  static void drawTextInRect(CQSchemRenderer *renderer, const QRectF &r, const QString &text);
  static void drawTextAtPoint(CQSchemRenderer *renderer, const QPointF &p, const QString &text);

 private:
  using Gates           = std::vector<CQGate *>;
  using PlacementGroups = std::vector<CQPlacementGroup *>;
  using Connections     = std::vector<CQConnection *>;

  bool             showConnectionText_ { true };
  bool             showGateText_       { true };
  Gates            gates_;
  Connections      connections_;
  CQPlacementGroup placementGroup_;
  QRectF           rect_;
  CQSchemRenderer  renderer_;
  QPointF          pressPoint_;
  bool             pressed_            { false };
  CQGate*          pressGate_          { nullptr };
  QPointF          movePoint_;
};

//---

class CQConnection {
 public:
  using Ports = std::vector<CQPort *>;

 public:
  CQConnection(const QString &name="") :
   name_(name) {
  }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  bool getValue() const { return value_; }
  void setValue(bool b);

  const Ports &inPorts() const { return inPorts_; }
  void addInPort(CQPort *p) { inPorts_.push_back(p); }

  const Ports &outPorts() const { return outPorts_; }
  void addOutPort(CQPort *p) { outPorts_.push_back(p); }

  CQBus *bus() const { return bus_; }
  void setBus(CQBus *bus) { bus_ = bus; }

  bool inside(const QPointF &p) const {
    return rect_.contains(p);
  }

  QColor penColor() const { return (getValue() ? Qt::green : Qt::white); }

  void draw(CQSchemRenderer *renderer) const;

  void drawLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2) const;

 private:
  QString        name_;
  bool           value_ { false };
  Ports          inPorts_;
  Ports          outPorts_;
  CQBus*         bus_   { nullptr };
  mutable QRectF rect_;
};

//---

class CQBus {
 public:
  CQBus(int n=8) :
   n_(n) {
    connections_.resize(n);
  }

  void addConnection(CQConnection *connection, int i) {
    assert(i >= 0 && i < n_);

    connections_[i] = connection;

    connection->setBus(this);
  }

  int connectionIndex(CQConnection *connection) {
    for (int i = 0; i < n_; ++i)
      if (connections_[i] == connection)
        return i;

    assert(false);

    return -1;
  }

 private:
  using Connections = std::vector<CQConnection *>;

  int         n_ { 8 };
  Connections connections_;
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
  enum class Direction {
    HORIZONTAL,
    VERTICAL,
    GRID
  };

  using Ports = std::vector<CQPort *>;

 public:
  CQGate(const QString &name) :
   name_(name) {
  }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  const Direction &direction() const { return direction_; }
  void setDirection(const Direction &v) { direction_ = v; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  void connect(const QString &name, CQConnection *connection);

  const Ports &inputs() const { return inputs_; }
  const Ports &outputs() const { return outputs_; }

  const QRectF &rect() const { return rect_; }
  void setRect(const QRectF &v) { rect_ = v; }

  CQPlacementGroup *placementGroup() const { return placementGroup_; }
  void setPlacementGroup(CQPlacementGroup *g) { placementGroup_ = g; }

  void addInputPort(const QString &name);
  void addOutputPort(const QString &name);

  CQPort *getPortByName(const QString &name) const;

  bool inside(const QPointF &p) const {
    return prect_.contains(p);
  }

  QColor penColor() const { return (isSelected() ? Qt::yellow : Qt::white); }

  virtual QSizeF calcSize() const;

  virtual bool exec() = 0;

  virtual void draw(CQSchemRenderer *renderer) const;

  void drawAnd(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const;

  void drawNotIndicator(CQSchemRenderer *renderer, double x, double y) const;

 protected:
  QString           name_;
  Direction         direction_      { Direction::VERTICAL };
  bool              selected_       { false };
  Ports             inputs_;
  Ports             outputs_;
  QRectF            rect_;
  mutable QRectF    prect_;
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

class CQMemoryGate : public CQGate {
 public:
  CQMemoryGate(const QString &name);

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

 private:
  bool state_ { false };
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

  bool exec() override;

  void draw(CQSchemRenderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }
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

    return QString("%1/%2/%3").arg(i3).arg(i2).arg(i1);
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

#endif
