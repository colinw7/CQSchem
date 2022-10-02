#ifndef CQSchem_H
#define CQSchem_H

#include <CDisplayRange2D.h>
#include <CDisplayTransform2D.h>
#include <QFrame>
#include <QPainter>
#include <set>

class QSplitter;
class QLabel;
class QToolButton;
class QPainter;
class QTimer;

namespace CQSchem {

enum class Direction {
  NONE,
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

enum class Alignment {
  LEFT,
  CENTER,
  RIGHT,
  HFILL,
  VFILL,
  FILL
};

inline Side otherSide(const Side &side) {
  if (side == Side::LEFT  ) return Side::RIGHT;
  if (side == Side::RIGHT ) return Side::RIGHT;
  if (side == Side::TOP   ) return Side::BOTTOM;
  if (side == Side::BOTTOM) return Side::TOP;
  return side;
}

struct SidePoint {
  QPoint    p;
  Side      side      { Side::NONE };
  Direction direction { Direction::NONE };

  SidePoint(const QPoint &p, const Side &side=Side::NONE,
            const Direction &direction=Direction::NONE) :
   p(p), side(side), direction(direction) {
  }
};

using SidePoints = std::vector<SidePoint>;

//---

class Schematic;
class Waveform;
class Gate;
class Port;
class Connection;
class Bus;
class PlacementGroup;

struct Renderer {
  Schematic*          schem           { nullptr };
  QPainter*           painter         { nullptr };
  QRect               prect;
  QRectF              rect;
  QRectF              placementRect;
  CDisplayRange2D     displayRange;
  CDisplayTransform2D displayTransform;
  bool                selected        { false };
  bool                inside          { false };
  QColor              connectionColor { 255, 255, 255, 128 };
  QColor              gateStrokeColor { 200, 200, 200, 128 };
  QColor              gateFillColor   { 80, 80, 100 };
  QColor              textColor       { 200, 200, 200 };
  QColor              selectColor     { Qt::yellow };
  QColor              insideColor     { Qt::cyan };

  Renderer() :
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

class PlacementGroup {
 public:
  enum class Placement {
    VERTICAL,
    HORIZONTAL,
    GRID
  };

  struct GateData {
    Gate*     gate      { nullptr };
    int       r         { -1 };
    int       c         { -1 };
    int       nr        { 1 };
    int       nc        { 1 };
    Alignment alignment { Alignment::CENTER };

    GateData(Gate *gate, int r=-1, int c=-1, int nr=1, int nc=1,
             Alignment alignment=Alignment::CENTER) :
     gate(gate), r(r), c(c), nr(nr), nc(nc), alignment(alignment) {
    }
  };

  struct PlacementGroupData {
    PlacementGroup *placementGroup { nullptr };
    int             r              { -1 };
    int             c              { -1 };
    int             nr             { 1 };
    int             nc             { 1 };
    Alignment       alignment      { Alignment::CENTER };

    PlacementGroupData(PlacementGroup *placementGroup, int r=-1, int c=-1,
                       int nr=1, int nc=1, Alignment alignment=Alignment::CENTER) :
     placementGroup(placementGroup), r(r), c(c), nr(nr), nc(nc), alignment(alignment) {
    }
  };

  using GateDatas       = std::vector<GateData>;
  using Gates           = std::vector<Gate *>;
  using Connections     = std::vector<Connection *>;
  using PlacementGroups = std::vector<PlacementGroupData>;
  using Buses           = std::vector<Bus *>;

 public:
  PlacementGroup(const Placement &placement=Placement::HORIZONTAL, int nr=-1, int nc=-1);

 ~PlacementGroup();

  const Placement &placement() const { return placement_; }
  void setPlacement(const Placement &v) { placement_ = v; }

  int numRows() const { return nr_; }
  void setNumRows(int i) { nr_ = i; }

  int numColumns() const { return nc_; }
  void setNumColumns(int i) { nc_ = i; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const GateDatas &gates() const { return gates_; }

  PlacementGroup *parent() const { return parentPlacementGroup_; }

  const PlacementGroups &placementGroups() const { return placementGroups_; }

  const QRectF &rect() const { updateRect(); return rect_; }
  void setRect(const QRectF &r);

  double area() const { return w_*h_; }

  const QString &expandName() const { return expandName_; }
  void setExpandName(const QString &v) { expandName_ = v; }

  const QString &collapseName() const { return collapseName_; }
  void setCollapseName(const QString &v) { collapseName_ = v; }

  void addGate(Gate *gate, int r=-1, int c=-1, int nr=1, int nc=1,
               Alignment alignment=Alignment::CENTER);

  void removeGate(Gate *gate);

  void addConnection(Connection *connection);
  void removeConnection(Connection *connection);

  void addBus(Bus *bus);
  void removeBus(Bus *bus);

  PlacementGroup *addPlacementGroup(const Placement &placement, int nr, int nc,
                                    int r1=-1, int c1=-1, int nr1=1, int nc1=1,
                                    Alignment alignment=Alignment::CENTER);

  void addPlacementGroup(PlacementGroup *placementGroup, int r=-1, int c=-1,
                         int nr=1, int nc=1, Alignment alignment=Alignment::CENTER);

  PlacementGroup *replacePlacementGroup(Schematic *schem, PlacementGroup *placementGroup);

  void hierGates(Gates &gates) const;
  void hierConnections(Connections &connections) const;
  void hierBuses(Buses &buses) const;

  bool inside(const QPointF &p) const { return prect_.contains(p); }

  QSizeF calcSize() const;

  void place();

  void draw(Renderer *renderer) const;

  void updateRect() const;

  QColor penColor(Renderer *renderer) const;

  PlacementGroup* nearestPlacementGroup(const QPointF &p) const;

 private:
  Placement       placement_            { Placement::HORIZONTAL };
  int             nr_                   { -1 };
  int             nc_                   { -1 };
  GateDatas       gates_;
  Connections     connections_;
  Buses           buses_;
  PlacementGroup* parentPlacementGroup_ { nullptr };
  PlacementGroups placementGroups_;
  QString         expandName_;
  QString         collapseName_;
  QRectF          rect_;
  bool            rectValid_            { false };
  mutable QRectF  prect_;
  double          w_                    { 1.0 };
  double          h_                    { 1.0 };
  bool            selected_             { false };
  mutable double  margin_               { 0.01 };
};

//---

class Window : public QFrame {
  Q_OBJECT

 public:
  Window(bool waveform);

  Schematic *schem() const { return schem_; }

  void setPos(const QPointF &pos);

  void redraw();

  QSize sizeHint() const override;

 private slots:
  void timerSlot();

  void connectionTextSlot(bool b);
  void gateTextSlot      (bool b);
  void portTextSlot      (bool b);

  void moveGateSlot      (bool b);
  void movePlacementSlot (bool b);
  void moveConnectionSlot(bool b);

  void connectionVisibleSlot    (bool b);
  void gateVisibleSlot          (bool b);
  void placementGroupVisibleSlot(bool b);

  void collapseBusSlot(bool b);

  void playSlot();
  void pauseSlot();
  void stepSlot();

 private:
  QSplitter*   splitter_    { nullptr };
  Schematic*   schem_       { nullptr };
  QLabel*      posLabel_    { nullptr };
  Waveform*    waveform_    { nullptr };
  QTimer*      timer_       { nullptr };
  bool         timerActive_ { false };
  QToolButton* playButton_  { nullptr };
  QToolButton* pauseButton_ { nullptr };
  QToolButton* stepButton_  { nullptr };
};

//---

class Schematic : public QFrame {
  Q_OBJECT

 public:
  enum class TextLinePos {
    START,
    END,
    MIDDLE
  };

  enum class TextAlign {
    LEFT,
    RIGHT,
    CENTER
  };

  using Gates           = std::vector<Gate *>;
  using PlacementGroups = std::vector<PlacementGroup *>;
  using Connections     = std::vector<Connection *>;
  using Buses           = std::vector<Bus *>;

 public:
  Schematic(Window *window);
 ~Schematic();

  //---

  void setWaveform(Waveform *waveform) { waveform_ = waveform; }

  //---

  bool isShowConnectionText() const { return showConnectionText_; }
  void setShowConnectionText(bool b) { showConnectionText_ = b; redraw(); }

  bool isShowGateText() const { return showGateText_; }
  void setShowGateText(bool b) { showGateText_ = b; redraw(); }

  bool isShowPortText() const { return showPortText_; }
  void setShowPortText(bool b) { showPortText_ = b; redraw(); }

  //---

  bool isMoveGate() const { return moveGate_; }
  void setMoveGate(bool b) { moveGate_ = b; }

  bool isMovePlacement() const { return movePlacement_; }
  void setMovePlacement(bool b) { movePlacement_ = b; }

  bool isMoveConnection() const { return moveConnection_; }
  void setMoveConnection(bool b) { moveConnection_ = b; }

  //---

  bool isConnectionVisible() const { return connectionVisible_; }
  void setConnectionVisible(bool b) { connectionVisible_ = b; redraw(); }

  bool isGateVisible() const { return gateVisible_; }
  void setGateVisible(bool b) { gateVisible_ = b; redraw(); }

  bool isPlacementGroupVisible() const { return placementGroupVisible_; }
  void setPlacementGroupVisible(bool b) { placementGroupVisible_ = b; redraw(); }

  //---

  bool isCollapseBus() const { return collapseBus_; }
  void setCollapseBus(bool b) { collapseBus_ = b; redraw(); }

  //---

  bool isDebugConnect() const { return debugConnect_; }
  void setDebugConnect(bool b) { debugConnect_ = b; }

  //---

  void clear();

  void deselectAll();

  //---

  bool execGate(const QString &name);
  bool execGate(PlacementGroup *parentGroup, const QString &name);

  void addNandGate       ();
  void addNotGate        ();
  void addAndGate        ();
  void addAnd3Gate       ();
  void addAnd4Gate       ();
  void addAnd8Gate       ();
  void addOrGate         ();
  void addOr8Gate        ();
  void addXorGate        ();
  void addMemoryGate     ();
  void addMemory8Gate    ();
  void addEnablerGate    ();
  void addRegisterGate   ();
  void addDecoder4Gate   ();
  void addDecoder8Gate   ();
  void addDecoder16Gate  ();
  void addDecoder256Gate ();
  void addLShiftGate     ();
  void addRShiftGate     ();
  void addInverterGate   ();
  void addAnderGate      ();
  void addOrerGate       ();
  void addXorerGate      ();
  void addAdderGate      ();
  void addAdder8Gate     ();
  void addComparatorGate ();
  void addComparator8Gate();
  void addBus0Gate       ();
  void addBus1Gate       ();
  void addAluGate        ();
  void addClkGate        (int delay=0, int cycle=0);
  void addClkESGate      ();
  void addStepperGate    ();

  void buildNotGate       ();
  void buildAndGate       ();
  void buildAnd3Gate      ();
  void buildAnd4Gate      ();
  void buildAnd8Gate      ();
  void buildOrGate        ();
  void buildOr8Gate       ();
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
  void buildBus0          ();
  void buildBus1          ();
  void buildRam256        ();
  void buildRam65536      ();
  void buildAlu           ();
  void buildClk           ();
  void buildClkES         ();
  void buildStepper       ();
  void buildControl1      ();
  void buildControl2      ();
  void buildControl3      ();
  void buildControl4      ();
  void buildControl5      ();
  void testConnection     ();

  Connection *addConnection(const QString &name);
  void removeConnection(Connection *connection);

  template<typename T>
  T *addGateT(const QString &name="") {
    T *gate = new T(name);

    gate->setSchem(this);

    addGate(gate);

    return gate;
  }

  template<typename T>
  T *addPlacementGateT(PlacementGroup *placementGroup, const QString &name,
                       const QString &expandName) {
    PlacementGroup *placementGroup1 =
      new PlacementGroup(PlacementGroup::Placement::HORIZONTAL);

    placementGroup1->setExpandName(expandName);

    T *gate = addGateT<T>(name);

    placementGroup1->addGate(gate);

    placementGroup->addPlacementGroup(placementGroup1);

    return gate;
  }

  template<typename T>
  T *addPlacementGateT(PlacementGroup *placementGroup, const QString &name,
                       const QString &expandName, int r, int c, int nr=1, int nc=1,
                       Alignment alignment=Alignment::CENTER) {
    PlacementGroup *placementGroup1 =
      new PlacementGroup(PlacementGroup::Placement::HORIZONTAL);

    placementGroup1->setExpandName(expandName);

    T *gate = addGateT<T>(name);

    placementGroup1->addGate(gate);

    placementGroup->addPlacementGroup(placementGroup1, r, c, nr, nc, alignment);

    return gate;
  }

  void addGate(Gate *gate);
  void removeGate(Gate *gate);

  Bus *addBus(const QString &name, int n);
  void removeBus(Bus *bus);

  PlacementGroup *addPlacementGroup(PlacementGroup::Placement placement=
                                     PlacementGroup::Placement::HORIZONTAL,
                                    int nr=-1, int nc=-1);

  void addPlacementGroup(PlacementGroup *placementGroup);

  void place();

  void calcBounds();

  bool exec();

  void test();

  void addTValue(const Connection *connection, bool b);

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

  Gate*           nearestGate          (const QPointF &p) const;
  PlacementGroup* nearestPlacementGroup(const QPointF &p) const;
  Connection*     nearestConnection    (const QPointF &p) const;

  Gate*           insideGate      () const { return insideGate_      ; }
  PlacementGroup* insidePlacement () const { return insidePlacement_ ; }
  Connection*     insideConnection() const { return insideConnection_; }

  void selectedGates(Gates &gates) const;

  void selectedConnections(Connections &connections) const;

  void selectedBuses(Buses &buses) const;

  void selectedPlacementGroups(PlacementGroups &placementGroups) const;

  static void drawConnection(Renderer *renderer, const QPointF &p1, const QPointF &p2);

  static void drawTextInRect(Renderer *renderer, const QRectF &r, const QString &text);

  static void drawTextOnLine(Renderer *renderer, const QPointF &p1, const QPointF &p2,
                             const QString &name, TextLinePos pos=TextLinePos::MIDDLE);

  static void drawTextAtPoint(Renderer *renderer, const QPointF &p, const QString &text,
                              TextAlign align=TextAlign::CENTER);

  static void drawLine(Renderer *renderer, const QPointF &p1, const QPointF &p2);

  void redraw();

  void resetObjs();

  QSize sizeHint() const override;

 private:
 private slots:
  void expandSlot();
  void collapseSlot();
  void placeSlot();

 private:
  Window*         window_                { nullptr };
  Waveform*       waveform_              { nullptr };
  bool            showConnectionText_    { true };
  bool            showGateText_          { true };
  bool            showPortText_          { false };
  bool            moveGate_              { true };
  bool            movePlacement_         { false };
  bool            moveConnection_        { false };
  bool            connectionVisible_     { true };
  bool            placementGroupVisible_ { false };
  bool            gateVisible_           { true };
  bool            collapseBus_           { false };
  Gates           gates_;
  Buses           buses_;
  Connections     connections_;
  PlacementGroup* placementGroup_        { nullptr };
  int             t_                     { 0 };
  QRectF          rect_;
  QImage          image_;
  bool            changed_;
  Renderer        renderer_;
  QPointF         pressPoint_;
  bool            pressed_               { false };
  Gate*           pressGate_             { nullptr };
  PlacementGroup* pressPlacement_        { nullptr };
  Connection*     pressConnection_       { nullptr };
  Gate*           insideGate_            { nullptr };
  PlacementGroup* insidePlacement_       { nullptr };
  Connection*     insideConnection_      { nullptr };
  QPointF         movePoint_;
  bool            debugConnect_          { false };
};

//---

class Waveform : public QFrame {
  Q_OBJECT

 public:
  Waveform(Schematic *schem);

  void addValue(const Connection *connection, int t, bool b);

  QSize sizeHint() const override;

 private:
  void paintEvent(QPaintEvent *) override;

 private:
  using Values    = std::map<int, bool>;
  using ConnInds  = std::map<const Connection *, int>;
  using IndConns  = std::map<int, const Connection *>;
  using IndValues = std::map<int, Values>;

  Schematic* schem_ { nullptr };
  ConnInds   connInds_;
  IndConns   indConns_;
  IndValues  indValues_;
};

//---

class Connection {
 public:
  struct Line {
    int     ind;
    QPointF start;
    QPointF end;

    Line(int ind, const QPointF &start, const QPointF &end) :
     ind(ind), start(start), end(end) {
    }

    bool contains(const QPointF &p) const {
      double dist;

      (void) pointDistance(p, dist);

      return (dist < 2);
    }

    bool pointDistance(const QPointF &p, double &dist) const {
      auto pointPointDistance = [](const QPointF &p1, const QPointF &p2) {
        double dx = p1.x() - p2.x();
        double dy = p1.y() - p2.y();

        return std::hypot(dx, dy);
      };

      double dx1 = end.x() - start.x();
      double dy1 = end.y() - start.y();

      double dx2 = p.x() - start.x();
      double dy2 = p.y() - start.y();

      double u1 = dx2*dx1 + dy2*dy1;
      double u2 = dx1*dx1 + dy1*dy1;

      if (u2 <= 0.0) {
        dist = pointPointDistance(p, start);
        return false;
      }

      double u = u1/u2;

      if      (u < 0.0) {
        dist = pointPointDistance(p, start);
        return false;
      }
      else if (u > 1.0) {
        dist = pointPointDistance(p, end);
        return false;
      }
      else {
        QPointF intersection = start + u*QPointF(dx1, dy1);

        dist = pointPointDistance(p, intersection);

        return true;
      }
    }
  };

  using Ports = std::vector<Port *>;
  using Lines = std::vector<Line>;

  class LinesData {
   public:
    struct ILine {
      QPoint start;
      QPoint end;

      ILine(const QPoint &start, const QPoint &end) :
       start(start), end(end) {
      }
    };

   public:
    LinesData() { }

    void addLine(const QPoint &p1, const QPoint &p2) {
      int dx = std::abs(p2.x() - p1.x());
      int dy = std::abs(p2.y() - p1.y());

      assert(dx == 0 || dy == 0);

      if (dy == 0) {
        int x1 = p1.x();
        int x2 = p2.x();

        if (x1 > x2)
          std::swap(x1, x2);

        xlines_[p1.y()][x1].insert(x2);
      }
      else {
        int y1 = p1.y();
        int y2 = p2.y();

        if (y1 > y2)
          std::swap(y1, y2);

        ylines_[p1.x()][y1].insert(y2);
      }
    }

    const Lines &lines() const {
      if (lines_.empty()) {
        int ind = 0;

        for (auto &p1 : xlines_) {
          int y = p1.first;

          IPairs xpairs;

          for (auto &p2 : p1.second) {
            int x1 = p2.first;
            int x2 = *p2.second.rbegin();

            xpairs.push_back(IPair(x1, x2));
          }

          assert(! xpairs.empty());

          auto p = xpairs.begin();

          IPair ipair1 = *p++;
          IPair ipair2 = ipair1;

          while (p != xpairs.end()) {
            ipair2 = *p++;

            if ((ipair2.first  >= ipair1.first && ipair2.first  <= ipair1.second) ||
                (ipair2.second >= ipair1.first && ipair2.second <= ipair1.second)) {
              ipair1 = IPair(std::min(ipair1.first , ipair2.first ),
                             std::max(ipair1.second, ipair2.second));
            }
            else {
              lines_.push_back(Line(ind++, QPointF(ipair1.first, y), QPointF(ipair1.second, y)));

              ipair1 = ipair2;
            }
          }

          lines_.push_back(Line(ind++, QPointF(ipair1.first, y), QPointF(ipair1.second, y)));
        }

        for (auto &p1 : ylines_) {
          int x = p1.first;

          IPairs ypairs;

          for (auto &p2 : p1.second) {
            int y1 = p2.first;
            int y2 = *p2.second.rbegin();

            ypairs.push_back(IPair(y1, y2));
          }

          assert(! ypairs.empty());

          auto p = ypairs.begin();

          IPair ipair1 = *p++;
          IPair ipair2 = ipair1;

          while (p != ypairs.end()) {
            ipair2 = *p++;

            if ((ipair2.first  >= ipair1.first && ipair2.first  <= ipair1.second) ||
                (ipair2.second >= ipair1.first && ipair2.second <= ipair1.second)) {
              ipair1 = IPair(std::min(ipair1.first , ipair2.first ),
                             std::max(ipair1.second, ipair2.second));
            }
            else {
              lines_.push_back(Line(ind++, QPointF(x, ipair1.first), QPointF(x, ipair1.second)));

              ipair1 = ipair2;
            }
          }

          lines_.push_back(Line(ind++, QPointF(x, ipair1.first), QPointF(x, ipair1.second)));
        }
      }

      return lines_;
    }

   private:
    using ILines   = std::vector<ILine>;
    using IValues  = std::set<int>;
    using IPair    = std::pair<int, int>;
    using IPairs   = std::vector<IPair>;
    using IValues1 = std::map<int, IValues>;
    using IValues2 = std::map<int, IValues1>;

    IValues2      xlines_;
    IValues2      ylines_;
    mutable Lines lines_;
  };

 public:
  Connection(const QString &name="");

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  Schematic *schem() { return schem_; }
  void setSchem(Schematic *p) { schem_ = p; }

  bool getValue() const { return value_; }
  void setValue(bool b);

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  bool isTraced() const { return traced_; }
  void setTraced(bool b) { traced_ = b; }

  const Ports &inPorts() const { return inPorts_; }
  void addInPort(Port *p) { inPorts_.push_back(p); }

  const Ports &outPorts() const { return outPorts_; }
  void addOutPort(Port *p) { outPorts_.push_back(p); }

  Bus *bus() const { return bus_; }
  void setBus(Bus *bus) { bus_ = bus; }

  const QRectF &prect() const { return prect_; }
  void setPRect(const QRectF &r) { prect_ = r; }

  bool isInput () const { return (inPorts_.empty() && ! outPorts_.empty()); }
  bool isOutput() const { return (! inPorts_.empty() && outPorts_.empty()); }

  bool anyPorts() const { return (! inPorts_.empty() || ! outPorts_.empty()); }

  bool isLR() const;

  bool isLeft() const;
  bool isTop () const;

  void merge(Connection *connection);

  void removePort(Port *port);

  bool inside(const QPointF &p) const;

  void draw(Renderer *renderer) const;

  QPointF imidPoint() const;
  QPointF omidPoint() const;
  QPointF midPoint() const;

  QColor penColor(Renderer *renderer) const;

 private:
  void calcSinglePointLines(Renderer *renderer, const SidePoints &points, Lines &lines) const;

  void calcSingleDirectionLines(Renderer *renderer, const SidePoints &points, Lines &lines) const;

  void calcLines(Renderer *renderer, const SidePoints &points, Lines &lines) const;

  void addConnectLines(const QPointF &p1, const QPointF &p2,
                       const QPointF &p3, const QPointF &p4) const;

  void connectPoints(const QPointF &p1, const QPointF &p2) const;
  void connectPoints(Lines &lines, const QPointF &p1, const QPointF &p2) const;

  void addLine(const QPointF &p1, const QPointF &p2) const;
  void addLine(Lines &lines, const QPointF &p1, const QPointF &p2) const;

  void drawLine(Renderer *renderer, const QPointF &p1, const QPointF &p2,
                bool showText=false) const;

 private:
  QString        name_;
  Schematic*     schem_    { nullptr };
  bool           value_    { false };
  bool           selected_ { false };
  bool           traced_   { false };
  Ports          inPorts_;
  Ports          outPorts_;
  Bus*           bus_      { nullptr };
  mutable QRectF prect_;
  mutable Lines  lines_;
};

//---

class Bus {
 public:
  enum class Position {
    START,
    MIDDLE,
    END
  };

 public:
  Bus(const QString &name="", int n=8);

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  int n() const { return n_; }

  Gate *gate() const { return gate_; }
  void setGate(Gate *gate) { gate_ = gate; }

  bool isFlipped() const { return flipped_; }
  void setFlipped(bool b) { flipped_ = b; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const Position &position() const { return position_; }
  double offset() const { return offset_; }

  void setPosition(const Position &position, double offset=0.0) {
    position_ = position; offset_ = offset;
  }

  void addConnection(Connection *connection, int i);

  int connectionIndex(Connection *connection);

  void draw(Renderer *renderer);

 private:
  using Connections = std::vector<Connection *>;

  QString     name_;
  int         n_            { 8 };
  Gate*       gate_         { nullptr };
  Connections connections_;
  bool        flipped_      { false };
  bool        selected_     { false };
  Position    position_     { Position::MIDDLE };
  double      offset_       { 0.0 };
};

//---

class Port {
 public:
  Port(const QString &name, const Direction &direction) :
   name_(name), direction_(direction) {
  }

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  const Direction &direction() const { return direction_; }
  void setDirection(const Direction &direction) { direction_ = direction; }

  const Side &side() const { return side_; }
  void setSide(const Side &v) { side_ = v; }

  const Gate *gate() const { return gate_; }
  void setGate(Gate *p) { gate_ = p; }

  bool getValue() const { return value_; }
  void setValue(bool b, bool propagate=true);

  Connection *connection() const { return connection_; }
  void setConnection(Connection *connection) { connection_ = connection; }

  const QPointF &pixelPos() const { return ppos_; }
  void setPixelPos(const QPointF &v) { ppos_ = v; }

  QPointF offsetPixelPos() const;

  Side calcSide() const;

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
  Gate*           gate_       { nullptr };
  bool            value_      { false };
  Connection*     connection_ { nullptr };
  mutable QPointF ppos_;
};

//---

class Gate {
 public:
  enum class Orientation {
    R0,
    R90,
    R180,
    R270
  };

  using Ports = std::vector<Port *>;

 public:
  Gate(const QString &name);

  virtual ~Gate();

  const QString &name() const { return name_; }
  void setName(const QString &v) { name_ = v; }

  Schematic *schem() { return schem_; }
  void setSchem(Schematic *p) { schem_ = p; }

  const Orientation &orientation() const { return orientation_; }
  void setOrientation(const Orientation &v) { orientation_ = v; }

  bool isFlipped() const { return flipped_; }
  void setFlipped(bool b) { flipped_ = b; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  void connect(const QString &name, Connection *connection);

  const Ports &inputs () const { return inputs_ ; }
  const Ports &outputs() const { return outputs_; }

  const QRectF &rect() const { return rect_; }
  void setRect(const QRectF &v) { rect_ = v; }

  PlacementGroup *placementGroup() const { return placementGroup_; }
  void setPlacementGroup(PlacementGroup *g) { placementGroup_ = g; }

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

  Port *getPortByName(const QString &name) const;

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

    auto x = std::vector<double>(uint(n));

    double xgap = this->xgap(px1, px2, n);

    if (! flip) {
      x[0] = px1 + 0.5*xgap;

      for (int i = 1; i < n; ++i)
        x[uint(i)] = x[uint(i - 1)] + xgap;
    }
    else {
      x[0] = px2 - 0.5*xgap;

      for (int i = 1; i < n; ++i)
        x[uint(i)] = x[uint(i - 1)] - xgap;
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

    auto y = std::vector<double>(uint(n));

    double ygap = this->ygap(py1, py2, n);

    if (! flip) {
      y[0] = py1 + 0.5*ygap;

      for (int i = 1; i < n; ++i)
        y[uint(i)] = y[uint(i - 1)] + ygap;
    }
    else {
      y[0] = py2 - 0.5*ygap;

      for (int i = 1; i < n; ++i)
        y[uint(i)] = y[uint(i - 1)] - ygap;
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

  virtual void draw(Renderer *renderer) const;

  void setBrush(Renderer *renderer) const;

  void drawRect(Renderer *renderer) const;

  void drawAnd(Renderer *renderer) const;
  void drawAnd(Renderer *renderer, double x1, double y1, double x2, double y2) const;

  void drawOr(Renderer *renderer) const;
  void drawOr(Renderer *renderer, double &x3, double &y3) const;
  void drawOr(Renderer *renderer, double x1, double y1, double x2, double y2,
              double &x3, double &y3) const;

  void drawXor(Renderer *renderer) const;
  void drawXor(Renderer *renderer, double x1, double y1, double x2, double y2) const;

  void drawNot(Renderer *renderer) const;

  void drawNotIndicator(Renderer *renderer) const;

  void drawAdder(Renderer *renderer) const;

  void placePorts(int ni=-1, int no=-1) const;
  void placePorts(double pix1, double piy1, double pix2, double piy2,
                  double pox1, double poy1, double pox2, double poy2, int ni=-1, int no=-1) const;

  void placePortOnSide(Port *port, const Side &side) const;

  void placePortsOnSide(Port **ports, int n, const Side &side) const;

  QColor penColor(Renderer *renderer) const;

  void initRect(Renderer *renderer) const;

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
  QString         name_;
  Schematic*      schem_          { nullptr };
  Orientation     orientation_    { Orientation::R0 };
  bool            flipped_        { false };
  bool            selected_       { false };
  Ports           inputs_;
  Ports           outputs_;
  QRectF          rect_;
  mutable QRectF  prect_;
  double          w_              { 1.0 };
  double          h_              { 0.8 };
  double          margin_         { 0.1 };
  PlacementGroup* placementGroup_ { nullptr };
};

//---

class NandGate : public Gate {
 public:
  NandGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

// inputs : a
// outputs: c
class NotGate : public Gate {
 public:
  NotGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class AndGate : public Gate {
 public:
  AndGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class And3Gate : public Gate {
 public:
  And3Gate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class And4Gate : public Gate {
 public:
  And4Gate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class And8Gate : public Gate {
 public:
  And8Gate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
};

//---

class OrGate : public Gate {
 public:
  OrGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class Or8Gate : public Gate {
 public:
  Or8Gate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
};

//---

class XorGate : public Gate {
 public:
  XorGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

// inputs : i, s
// outputs: o
class MemoryGate : public Gate {
 public:
  MemoryGate(const QString &name);

  const Side &sside() const { return sside_; }
  void setSSide(const Side &s) { sside_ = s; }

  bool exec() override;

  void draw(Renderer *renderer) const override;

 private:
  bool state_ { false };
  Side sside_ { Side::LEFT };
};

//---

class Memory8Gate : public Gate {
 public:
  Memory8Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[8] {};
};

//---

class EnablerGate : public Gate {
 public:
  EnablerGate(const QString &name);

  const Side &eside() const { return eside_; }
  void setESide(const Side &s) { eside_ = s; }

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  Side eside_ { Side::LEFT };
};

//---

class RegisterGate : public Gate {
 public:
  RegisterGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[8] {};
};

//---

class Decoder4Gate : public Gate {
 public:
  Decoder4Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

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

class Decoder8Gate : public Gate {
 public:
  Decoder8Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

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

class Decoder16Gate : public Gate {
 public:
  Decoder16Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

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
    int i4 = (i & 8) >> 3;

    return QString("%1/%2/%3/%4").arg(i4).arg(i3).arg(i2).arg(i1);
  }
};

//---

class Decoder256Gate : public Gate {
 public:
  Decoder256Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

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

class LShiftGate : public Gate {
 public:
  LShiftGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[9];
};

//---

class RShiftGate : public Gate {
 public:
  RShiftGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }

 private:
  bool state_[9];
};

//---

class InverterGate : public Gate {
 public:
  InverterGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("a%1").arg(i); }
  static QString oname(int i) { return QString("c%1").arg(i); }
};

//---

class AnderGate : public Gate {
 public:
  AnderGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }
};

//---

class OrerGate : public Gate {
 public:
  OrerGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }
};

//---

class XorerGate : public Gate {
 public:
  XorerGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }
};

//---

class AdderGate : public Gate {
 public:
  AdderGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class Adder8Gate : public Gate {
 public:
  Adder8Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }
};

//---

class ComparatorGate : public Gate {
 public:
  ComparatorGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;
};

//---

class Comparator8Gate : public Gate {
 public:
  Comparator8Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }
};

//---

class Bus0Gate : public Gate {
 public:
  Bus0Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
};

//---

class Bus1Gate : public Gate {
 public:
  Bus1Gate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString iname(int i) { return QString("i%1").arg(i); }
  static QString oname(int i) { return QString("o%1").arg(i); }
};

//---

class AluGate : public Gate {
 public:
  AluGate(const QString &name);

  bool exec() override;

  void draw(Renderer *renderer) const override;

  static QString aname(int i) { return QString("a%1").arg(i); }
  static QString bname(int i) { return QString("b%1").arg(i); }
  static QString cname(int i) { return QString("c%1").arg(i); }

  static QString opname(int i) { return QString("op%1").arg(i); }
};

//---

// outputs: clk
class ClkGate : public Gate {
 public:
  ClkGate(const QString &name="");

  void setDelay(int delay) { delay_ = delay; delay1_ = delay_; }
  void setCycle(int cycle) { cycle_ = cycle; cycle1_ = cycle_; }

  bool exec() override;

  void draw(Renderer *renderer) const override;

 private:
  bool state_  { false };
  int  delay_  { 0 };
  int  cycle_  { 0 };
  int  delay1_ { 0 };
  int  cycle1_ { 0 };
};

//---

class ClkESGate : public Gate {
 public:
  ClkESGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;

 private:
  bool state1_  { false };
  int  delay1_  { 0 };
  int  cycle1_  { 8 };
  int  delay1t_ { 0 };
  int  cycle1t_ { 0 };

  bool state2_  { false };
  int  delay2_  { 4 };
  int  cycle2_  { 8 };
  int  delay2t_ { 0 };
  int  cycle2t_ { 0 };
};

//---

class StepperGate : public Gate {
 public:
  StepperGate(const QString &name="");

  bool exec() override;

  void draw(Renderer *renderer) const override;

 private:
  bool b_     { false };
  int  state_ { 0 };
};

}

#endif
