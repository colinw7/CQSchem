#include <CQSchem.h>
#include <QApplication>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <cassert>

#include <svg/connection_text_svg.h>
#include <svg/gate_text_svg.h>
#include <svg/move_connection_svg.h>
#include <svg/move_gate_svg.h>
#include <svg/move_placement_svg.h>
#include <svg/port_text_svg.h>
#include <svg/connection_visible_svg.h>
#include <svg/gate_visible_svg.h>
#include <svg/placement_group_visible_svg.h>
#include <svg/collapse_bus_svg.h>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQSchemWindow *window = new CQSchemWindow;

  CQSchem *schem = window->schem();

  bool test = false;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg = &argv[i][1];

      if (schem->execGate(arg.c_str()))
        continue;

      if (arg == "test")
        test = true;
      else
        std::cerr << "Invalid arg '-" << arg << "'\n";
    }
  }

  schem->place();

  schem->exec();

  if (test) {
    schem->test();
  }
  else {
    window->show();

    app.exec();
  }
}

//------

CQSchemWindow::
CQSchemWindow()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  //----

  QFrame *controlFrame = new QFrame;
  controlFrame->setObjectName("controlFrame");

  controlFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  layout->addWidget(controlFrame);

  QHBoxLayout *controlLayout = new QHBoxLayout(controlFrame);

  //---

  schem_ = new CQSchem(this);

  schem_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  layout->addWidget(schem_);

  //--

  auto addToolButton = [&](const QString &name, const QString &iconName, bool checked,
                           const QString &tip, const char *slotName) {
    QToolButton *button = new QToolButton;

    button->setObjectName(name);
    button->setIcon(CQPixmapCacheInst->getIcon(iconName));
    button->setIconSize(QSize(32, 32));
//  button->setText(text);
    button->setAutoRaise(true);
    button->setCheckable(true);
    button->setChecked(checked);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked(bool)), this, slotName);

    return button;
  };

  QToolButton *connectionTextButton =
    addToolButton("connectionText", "CONNECTION_TEXT", schem_->isShowConnectionText(),
                  "Connection Text", SLOT(connectionTextSlot(bool)));
  QToolButton *gateTextButton =
    addToolButton("gateText"      , "GATE_TEXT"      , schem_->isShowGateText(),
                  "Gate Text", SLOT(gateTextSlot(bool)));
  QToolButton *portTextButton =
    addToolButton("portText"      , "PORT_TEXT"      , schem_->isShowPortText(),
                  "Port Text", SLOT(portTextSlot(bool)));

  QToolButton *moveGateButton =
    addToolButton("moveGate"      , "MOVE_GATE"      , schem_->isMoveGate(),
                  "Move Gate", SLOT(moveGateSlot(bool)));
  QToolButton *movePlacementButton =
    addToolButton("movePlacement" , "MOVE_PLACEMENT" , schem_->isMovePlacement(),
                  "Move Placement", SLOT(movePlacementSlot(bool)));
  QToolButton *moveConnectionButton =
    addToolButton("moveConnection", "MOVE_CONNECTION", schem_->isMoveConnection(),
                  "Move Connection", SLOT(moveConnectionSlot(bool)));

  QToolButton *connectionVisibleButton =
    addToolButton("connectionVisible", "CONNECTION_VISIBLE",
                  schem_->isConnectionVisible(), "Connection Visible",
                  SLOT(connectionVisibleSlot(bool)));
  QToolButton *gateVisibleButton =
    addToolButton("gateVisible", "GATE_VISIBLE",
                  schem_->isGateVisible(), "Gate Visible",
                  SLOT(gateVisibleSlot(bool)));
  QToolButton *placementGroupVisibleButton =
    addToolButton("placementGroupVisible", "PLACEMENT_GROUP_VISIBLE",
                  schem_->isPlacementGroupVisible(), "Placement Group Visible",
                  SLOT(placementGroupVisibleSlot(bool)));

  QToolButton *collapseBusButton =
    addToolButton("collapseBus", "COLLAPSE_BUS",
                  schem_->isCollapseBus(), "Collapse Bus",
                  SLOT(collapseBusSlot(bool)));

  controlLayout->addWidget(connectionTextButton);
  controlLayout->addWidget(gateTextButton);
  controlLayout->addWidget(portTextButton);

  controlLayout->addWidget(moveGateButton);
  controlLayout->addWidget(movePlacementButton);
  controlLayout->addWidget(moveConnectionButton);

  controlLayout->addWidget(connectionVisibleButton);
  controlLayout->addWidget(gateVisibleButton);
  controlLayout->addWidget(placementGroupVisibleButton);

  controlLayout->addWidget(collapseBusButton);

  controlLayout->addStretch(1);

  posLabel_ = new QLabel;

  controlLayout->addWidget(posLabel_);
}

void
CQSchemWindow::
setPos(const QPointF &pos)
{
  posLabel_->setText(QString("%1,%2").arg(pos.x()).arg(pos.y()));
}

void
CQSchemWindow::
connectionTextSlot(bool b)
{
  schem_->setShowConnectionText(b);
}

void
CQSchemWindow::
gateTextSlot(bool b)
{
  schem_->setShowGateText(b);
}

void
CQSchemWindow::
portTextSlot(bool b)
{
  schem_->setShowPortText(b);
}

void
CQSchemWindow::
moveGateSlot(bool b)
{
  schem_->setMoveGate(b);

  if (b) {
    schem_->setMovePlacement (! b);
    schem_->setMoveConnection(! b);
  }
}

void
CQSchemWindow::
movePlacementSlot(bool b)
{
  schem_->setMovePlacement(b);

  if (b) {
    schem_->setMoveGate      (! b);
    schem_->setMoveConnection(! b);
  }
}

void
CQSchemWindow::
moveConnectionSlot(bool b)
{
  schem_->setMoveConnection(b);

  if (b) {
    schem_->setMoveGate     (! b);
    schem_->setMovePlacement(! b);
  }
}

void
CQSchemWindow::
connectionVisibleSlot(bool b)
{
  schem_->setConnectionVisible(b);

  update();
}

void
CQSchemWindow::
gateVisibleSlot(bool b)
{
  schem_->setGateVisible(b);

  update();
}

void
CQSchemWindow::
CQSchemWindow::placementGroupVisibleSlot(bool b)
{
  schem_->setPlacementGroupVisible(b);

  update();
}

void
CQSchemWindow::
collapseBusSlot(bool b)
{
  schem_->setCollapseBus(b);

  update();
}

QSize
CQSchemWindow::
sizeHint() const
{
  return QSize(800, 800);
}

//------

CQSchem::
CQSchem(CQSchemWindow *window) :
 window_(window)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  //---

#if 0
  QFont font = this->font();

  double s = font.pointSizeF();

  font.setPointSizeF(s*1.5);

  setFont(font);
#endif

  placementGroup_ = new CQPlacementGroup;
}

CQSchem::
~CQSchem()
{
  clear();
}

void
CQSchem::
clear()
{
  for (auto &gate : gates_)
    delete gate;

  gates_.clear();

  for (auto &bus : buses_)
    delete bus;

  buses_.clear();

  for (auto &connection : connections_)
    delete connection;

  connections_.clear();

  delete placementGroup_;

  placementGroup_ = new CQPlacementGroup;

  rect_ = QRectF();

  pressGate_        = nullptr;
  pressPlacement_   = nullptr;
  insideGate_       = nullptr;
  insidePlacement_  = nullptr;
  insideConnection_ = nullptr;
}

bool
CQSchem::
execGate(const QString &name)
{
  if      (name == "nand"       ) addNandGate();
  else if (name == "not"        ) addNotGate();
  else if (name == "and"        ) addAndGate();
  else if (name == "and3"       ) addAnd3Gate();
  else if (name == "and4"       ) addAnd4Gate();
  else if (name == "and8"       ) addAnd8Gate();
  else if (name == "or"         ) addOrGate();
  else if (name == "or8"        ) addOr8Gate();
  else if (name == "xor"        ) addXorGate();
  else if (name == "memory"     ) addMemoryGate();
  else if (name == "memory8"    ) addMemory8Gate();
  else if (name == "enabler"    ) addEnablerGate();
  else if (name == "register"   ) addRegisterGate();
  else if (name == "decoder4"   ) addDecoder4Gate();
  else if (name == "decoder8"   ) addDecoder8Gate();
  else if (name == "decoder16"  ) addDecoder16Gate();
  else if (name == "decoder256" ) addDecoder256Gate();
  else if (name == "lshift"     ) addLShiftGate();
  else if (name == "rshift"     ) addRShiftGate();
  else if (name == "inverter"   ) addInverterGate();
  else if (name == "ander"      ) addAnderGate();
  else if (name == "orer"       ) addOrerGate();
  else if (name == "xorer"      ) addXorerGate();
  else if (name == "adder"      ) addAdderGate();
  else if (name == "adder8"     ) addAdder8Gate();
  else if (name == "comparator" ) addComparatorGate();
  else if (name == "comparator8") addComparator8Gate();
  else if (name == "bus0"       ) addBus0Gate();
  else if (name == "bus1"       ) addBus1Gate();
  else if (name == "alu"        ) addAluGate();

  else if (name == "build_not"        ) buildNotGate();
  else if (name == "build_and"        ) buildAndGate();
  else if (name == "build_and3"       ) buildAnd3Gate();
  else if (name == "build_and4"       ) buildAnd4Gate();
  else if (name == "build_and8"       ) buildAnd8Gate();
  else if (name == "build_or"         ) buildOrGate();
  else if (name == "build_or8"        ) buildOr8Gate();
  else if (name == "build_xor"        ) buildXorGate();
  else if (name == "build_memory"     ) buildMemoryGate();
  else if (name == "build_memory8"    ) buildMemory8Gate();
  else if (name == "build_enabler"    ) buildEnablerGate();
  else if (name == "build_register"   ) buildRegisterGate();
  else if (name == "build_decoder4"   ) buildDecoder4Gate();
  else if (name == "build_decoder8"   ) buildDecoder8Gate();
  else if (name == "build_decoder16"  ) buildDecoder16Gate();
  else if (name == "build_decoder256" ) buildDecoder256Gate();
  else if (name == "build_lshift"     ) buildLShift();
  else if (name == "build_rshift"     ) buildRShift();
  else if (name == "build_inverter"   ) buildInverter();
  else if (name == "build_ander"      ) buildAnder();
  else if (name == "build_orer"       ) buildOrer();
  else if (name == "build_xorer"      ) buildXorer();
  else if (name == "build_adder"      ) buildAdder();
  else if (name == "build_adder8"     ) buildAdder8();
  else if (name == "build_comparator" ) buildComparator();
  else if (name == "build_comparator8") buildComparator8();
  else if (name == "build_bus0"       ) buildBus0();
  else if (name == "build_bus1"       ) buildBus1();
  else if (name == "build_ram256"     ) buildRam256();
  else if (name == "build_ram65536"   ) buildRam65536();
  else if (name == "build_alu"        ) buildAlu();

  else return false;

  return true;
}

void
CQSchem::
addNandGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNandGate *gate = addGateT<CQNandGate>("nand");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addNotGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_not");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNotGate *gate = addGateT<CQNotGate>("not");

  placementGroup->addGate(gate);

  CQConnection *in  = addPlacementConn("a");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in );
  gate->connect("c", out);
}

void
CQSchem::
addAndGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAndGate *gate = addGateT<CQAndGate>("and");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addAnd3Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and3");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAnd3Gate *gate = addGateT<CQAnd3Gate>("and3");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *in3 = addPlacementConn("c");
  CQConnection *out = addPlacementConn("d");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", out);
}

void
CQSchem::
addAnd4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAnd4Gate *gate = addGateT<CQAnd4Gate>("and4");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *in3 = addPlacementConn("c");
  CQConnection *in4 = addPlacementConn("d");
  CQConnection *out = addPlacementConn("e");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", in4);
  gate->connect("e", out);
}

void
CQSchem::
addAnd8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQAnd8Gate *gate = addGateT<CQAnd8Gate>("and8");

  placementGroup->addGate(gate);

  CQConnection *in[8];

  CQBus *ibus = addPlacementBus("i", 8);

  for (int i = 0; i < 8; ++i) {
    in[i] = addPlacementConn(CQAnd8Gate::iname(i));

    ibus->addConnection(in[i], i);
  }

  CQConnection *out = addPlacementConn("o");

  for (int i = 0; i < 8; ++i)
    gate->connect(CQAnd8Gate::iname(i), in[i]);

  gate->connect("o", out);
}

void
CQSchem::
addOrGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_or");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQOrGate *gate = addGateT<CQOrGate>("or");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addOr8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_or8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQOr8Gate *gate = addGateT<CQOr8Gate>("or8");

  placementGroup->addGate(gate);

  CQConnection *in[8];

  CQBus *ibus = addPlacementBus("i", 8);

  for (int i = 0; i < 8; ++i) {
    in[i] = addPlacementConn(CQOr8Gate::iname(i));

    ibus->addConnection(in[i], i);
  }

  CQConnection *out = addPlacementConn("o");

  for (int i = 0; i < 8; ++i)
    gate->connect(CQOr8Gate::iname(i), in[i]);

  gate->connect("o", out);
}

void
CQSchem::
addXorGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_xor");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQXorGate *gate = addGateT<CQXorGate>("xor");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addMemoryGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_memory");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQMemoryGate *gate = addGateT<CQMemoryGate>("M");

  placementGroup->addGate(gate);

  CQConnection *in1 = addPlacementConn("i");
  CQConnection *in2 = addPlacementConn("s");
  CQConnection *out = addPlacementConn("o");

  gate->connect("i", in1);
  gate->connect("s", in2);
  gate->connect("o", out);
}

void
CQSchem::
addMemory8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_memory8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQMemory8Gate *gate = addGateT<CQMemory8Gate>("B");

  placementGroup->addGate(gate);

  CQConnection *cons = addPlacementConn("s");

  gate->connect("s", cons);

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }
}

void
CQSchem::
addEnablerGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_enabler");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQEnablerGate *gate = addGateT<CQEnablerGate>("E");

  placementGroup->addGate(gate);

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQEnablerGate::iname(i);
    QString oname = CQEnablerGate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }

  CQConnection *cone = addPlacementConn("e");

  gate->connect("e", cone);
}

void
CQSchem::
addRegisterGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQRegisterGate *gate = addGateT<CQRegisterGate>("R");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }
}

void
CQSchem::
addDecoder4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQDecoder4Gate *gate = addGateT<CQDecoder4Gate>("2x4");

  placementGroup->addGate(gate);

  for (int i = 0; i < 2; ++i) {
    QString iname = CQDecoder4Gate::iname(i);

    CQConnection *con = addPlacementConn(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 4; ++i) {
    QString oname = CQDecoder4Gate::oname(i);

    CQConnection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQDecoder8Gate *gate = addGateT<CQDecoder8Gate>("3x8");

  placementGroup->addGate(gate);

  for (int i = 0; i < 3; ++i) {
    QString iname = CQDecoder8Gate::iname(i);

    CQConnection *con = addPlacementConn(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 8; ++i) {
    QString oname = CQDecoder8Gate::oname(i);

    CQConnection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder16Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder16");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQDecoder16Gate *gate = addGateT<CQDecoder16Gate>("4x16");

  placementGroup->addGate(gate);

  for (int i = 0; i < 4; ++i) {
    QString iname = CQDecoder16Gate::iname(i);

    CQConnection *coni = addPlacementConn(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 16; ++i) {
    QString oname = CQDecoder16Gate::oname(i);

    CQConnection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder256Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQDecoder256Gate *gate = addGateT<CQDecoder256Gate>("8x256");

  placementGroup->addGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQDecoder256Gate::iname(i);

    CQConnection *coni = addPlacementConn(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 256; ++i) {
    QString oname = CQDecoder256Gate::oname(i);

    CQConnection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addLShiftGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_lshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQLShiftGate *gate = addGateT<CQLShiftGate>("SHL");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *icon[8];
  CQConnection *ocon[8];

  gate->connect("shift_in" , addPlacementConn("shift_in" ));
  gate->connect("shift_out", addPlacementConn("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(CQLShiftGate::iname(i));
    ocon[i] = addPlacementConn(CQLShiftGate::oname(i));

    gate->connect(CQLShiftGate::iname(i), icon[i]);
    gate->connect(CQLShiftGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
CQSchem::
addRShiftGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_rshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQRShiftGate *gate = addGateT<CQRShiftGate>("SHR");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *icon[8];
  CQConnection *ocon[8];

  gate->connect("shift_in" , addPlacementConn("shift_in" ));
  gate->connect("shift_out", addPlacementConn("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(CQRShiftGate::iname(i));
    ocon[i] = addPlacementConn(CQRShiftGate::oname(i));

    gate->connect(CQRShiftGate::iname(i), icon[i]);
    gate->connect(CQRShiftGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
CQSchem::
addInverterGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQInverterGate *gate = addGateT<CQInverterGate>("INV");

  placementGroup->addGate(gate);

  CQBus *ibus = addPlacementBus("a", 8);
  CQBus *obus = addPlacementBus("c", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  CQConnection *icon[8];
  CQConnection *ocon[8];

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(CQInverterGate::iname(i));
    ocon[i] = addPlacementConn(CQInverterGate::oname(i));

    gate->connect(CQInverterGate::iname(i), icon[i]);
    gate->connect(CQInverterGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
CQSchem::
addAnderGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_ander");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQAnderGate *gate = addGateT<CQAnderGate>("AND");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(CQAnderGate::aname(i));
    bcon[i] = addPlacementConn(CQAnderGate::bname(i));
    ccon[i] = addPlacementConn(CQAnderGate::cname(i));

    gate->connect(CQAnderGate::aname(i), acon[i]);
    gate->connect(CQAnderGate::bname(i), bcon[i]);
    gate->connect(CQAnderGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
CQSchem::
addOrerGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_orer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQOrerGate *gate = addGateT<CQOrerGate>("AND");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(CQOrerGate::aname(i));
    bcon[i] = addPlacementConn(CQOrerGate::bname(i));
    ccon[i] = addPlacementConn(CQOrerGate::cname(i));

    gate->connect(CQOrerGate::aname(i), acon[i]);
    gate->connect(CQOrerGate::bname(i), bcon[i]);
    gate->connect(CQOrerGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
CQSchem::
addXorerGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_xorer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQXorerGate *gate = addGateT<CQXorerGate>("AND");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(CQXorerGate::aname(i));
    bcon[i] = addPlacementConn(CQXorerGate::bname(i));
    ccon[i] = addPlacementConn(CQXorerGate::cname(i));

    gate->connect(CQXorerGate::aname(i), acon[i]);
    gate->connect(CQXorerGate::bname(i), bcon[i]);
    gate->connect(CQXorerGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
CQSchem::
addAdderGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_adder");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAdderGate *gate = addGateT<CQAdderGate>("adder");

  placementGroup->addGate(gate);

  gate->connect("a"        , addPlacementConn("a"        ));
  gate->connect("b"        , addPlacementConn("b"        ));
  gate->connect("carry_in" , addPlacementConn("carry_in" ));
  gate->connect("carry_out", addPlacementConn("carry_out"));
  gate->connect("sum"      , addPlacementConn("sum"      ));
}

void
CQSchem::
addAdder8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_adder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQAdder8Gate *gate = addGateT<CQAdder8Gate>("ADD");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString aname = CQAdder8Gate::aname(i);
    QString bname = CQAdder8Gate::bname(i);
    QString cname = CQAdder8Gate::cname(i);

    CQConnection *acon = addPlacementConn(aname);
    CQConnection *bcon = addPlacementConn(bname);
    CQConnection *ccon = addPlacementConn(cname);

    gate->connect(aname, acon);
    gate->connect(bname, bcon);
    gate->connect(cname, ccon);

    abus->addConnection(acon, i);
    bbus->addConnection(bcon, i);
    cbus->addConnection(ccon, i);
  }

  gate->connect("carry_in" , addPlacementConn("carry_in" ));
  gate->connect("carry_out", addPlacementConn("carry_out"));
}

void
CQSchem::
addComparatorGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_comparator");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQComparatorGate *gate = addGateT<CQComparatorGate>("XOR");

  placementGroup->addGate(gate);

  gate->connect("a"        , addPlacementConn("a"       ));
  gate->connect("b"        , addPlacementConn("b"       ));
  gate->connect("c"        , addPlacementConn("c"       ));
  gate->connect("a_larger" , addPlacementConn("a_larger"));
  gate->connect("equal"    , addPlacementConn("equal"   ));
}

void
CQSchem::
addComparator8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_comparator8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQComparator8Gate *gate = addGateT<CQComparator8Gate>("XOR");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString aname = CQComparator8Gate::aname(i);
    QString bname = CQComparator8Gate::bname(i);
    QString cname = CQComparator8Gate::cname(i);

    CQConnection *acon = addPlacementConn(aname);
    CQConnection *bcon = addPlacementConn(bname);
    CQConnection *ccon = addPlacementConn(cname);

    gate->connect(aname, acon);
    gate->connect(bname, bcon);
    gate->connect(cname, ccon);

    abus->addConnection(acon, i);
    bbus->addConnection(bcon, i);
    cbus->addConnection(ccon, i);
  }

  gate->connect("a_larger", addPlacementConn("a_larger"));
  gate->connect("equal"   , addPlacementConn("equal"   ));
}

void
CQSchem::
addBus0Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_bus0");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus0Gate *gate = addGateT<CQBus0Gate>("Z");

  placementGroup->addGate(gate);

  CQBus *ibus = addPlacementBus("i", 8);

  ibus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    CQConnection *icon = addPlacementConn(CQBus0Gate::iname(i));

    gate->connect(CQBus0Gate::iname(i), icon);

    ibus->addConnection(icon, i);
  }

  gate->connect("zero", addPlacementConn("zero"));
}

void
CQSchem::
addBus1Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_bus1");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus1Gate *gate = addGateT<CQBus1Gate>("BUS1");

  placementGroup->addGate(gate);

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    CQConnection *icon = addPlacementConn(CQBus1Gate::iname(i));
    CQConnection *ocon = addPlacementConn(CQBus1Gate::oname(i));

    gate->connect(CQBus1Gate::iname(i), icon);
    gate->connect(CQBus1Gate::oname(i), ocon);

    ibus->addConnection(icon, i);
    obus->addConnection(ocon, i);
  }

  gate->connect("bus1", addPlacementConn("bus1"));
}

void
CQSchem::
addAluGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_alu");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQAluGate *gate = addGateT<CQAluGate>("ALU");

  placementGroup->addGate(gate);

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    CQConnection *acon = addPlacementConn(CQAluGate::aname(i));
    CQConnection *bcon = addPlacementConn(CQAluGate::bname(i));
    CQConnection *ccon = addPlacementConn(CQAluGate::cname(i));

    gate->connect(CQAluGate::aname(i), acon);
    gate->connect(CQAluGate::bname(i), bcon);
    gate->connect(CQAluGate::cname(i), ccon);

    abus->addConnection(acon, i);
    bbus->addConnection(bcon, i);
    cbus->addConnection(ccon, i);
  }

  gate->connect("carry_in" , addPlacementConn("carry_in" ));
  gate->connect("carry_out", addPlacementConn("carry_out"));
  gate->connect("a_larger" , addPlacementConn("a_larger" ));
  gate->connect("equal"    , addPlacementConn("equal"    ));
  gate->connect("zero"     , addPlacementConn("zero"     ));

  for (int i = 0; i < 3; ++i) {
    QString opname = CQAluGate::opname(i);

    gate->connect(opname, addPlacementConn(opname));
  }
}

void
CQSchem::
buildNotGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("not");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNandGate *gate = addGateT<CQNandGate>();

  placementGroup->addGate(gate);

  CQConnection *in  = addPlacementConn("a");
  CQConnection *out = addPlacementConn("c");

  gate->connect("a", in );
  gate->connect("b", in );
  gate->connect("c", out);
}

void
CQSchem::
buildAndGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("and");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNandGate *nandGate = addGateT<CQNandGate>();

  placementGroup->addGate(nandGate);

  CQConnection *in1  = addPlacementConn("a");
  CQConnection *in2  = addPlacementConn("b");
  CQConnection *out1 = addPlacementConn("x");

  nandGate->connect("a", in1 );
  nandGate->connect("b", in2 );
  nandGate->connect("c", out1);

  CQNotGate *notGate = addGateT<CQNotGate>();

  placementGroup->addGate(notGate);

  CQConnection *out2 = addPlacementConn("c");

  notGate->connect("a", out1);
  notGate->connect("c", out2);
}

void
CQSchem::
buildAnd3Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 2);

  placementGroup->setCollapseName("and3");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAndGate *andGate1 = addGateT<CQAndGate>();
  CQAndGate *andGate2 = addGateT<CQAndGate>();

  placementGroup->addGate(andGate1, 1, 0);
  placementGroup->addGate(andGate2, 0, 1);

  CQConnection *in1  = addPlacementConn("a");
  CQConnection *in2  = addPlacementConn("b");
  CQConnection *in3  = addPlacementConn("c");
  CQConnection *out1 = addPlacementConn("t");
  CQConnection *out2 = addPlacementConn("d");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);
}

void
CQSchem::
buildAnd4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("and4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAndGate *andGate1 = addGateT<CQAndGate>();
  CQAndGate *andGate2 = addGateT<CQAndGate>();
  CQAndGate *andGate3 = addGateT<CQAndGate>();

  placementGroup->addGate(andGate1, 2, 0);
  placementGroup->addGate(andGate2, 1, 1);
  placementGroup->addGate(andGate3, 0, 2);

  CQConnection *in1  = addPlacementConn("a");
  CQConnection *in2  = addPlacementConn("b");
  CQConnection *in3  = addPlacementConn("c");
  CQConnection *in4  = addPlacementConn("d");
  CQConnection *out1 = addPlacementConn("t1");
  CQConnection *out2 = addPlacementConn("t2");
  CQConnection *out3 = addPlacementConn("e");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);

  andGate3->connect("a", out2);
  andGate3->connect("b", in4 );
  andGate3->connect("c", out3);
}

void
CQSchem::
buildAnd8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 7, 7);

  placementGroup->setCollapseName("and8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQAndGate *andGate[7];

  for (int i = 0; i < 7; ++i) {
    andGate[i] = addGateT<CQAndGate>();

    placementGroup->addGate(andGate[i], 6 - i, i);
  }

  CQConnection *in[8];

  for (int i = 0; i < 8; ++i)
    in[i] = addPlacementConn(QString("a%1").arg(i));

  CQConnection *out[7];

  for (int i = 0; i < 7; ++i)
    out[i] = addPlacementConn(QString("t%1").arg(i));

  for (int i = 0; i < 7; ++i) {
    if (i == 0)
      andGate[i]->connect("a", in[i]);
    else
      andGate[i]->connect("a", out[i - 1]);

    andGate[i]->connect("b", in [i + 1]);
    andGate[i]->connect("c", out[i    ]);
  }
}

void
CQSchem::
buildOrGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 2);

  placementGroup->setCollapseName("or");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNotGate  *notGate1 = addGateT<CQNotGate>();
  CQNotGate  *notGate2 = addGateT<CQNotGate>();
  CQNandGate *nandGate = addGateT<CQNandGate>();

  placementGroup->addGate(notGate1, 1, 0);
  placementGroup->addGate(notGate2, 0, 0);
  placementGroup->addGate(nandGate, 0, 1, 2, 1);

  CQConnection *in1 = addPlacementConn("a");
  CQConnection *in2 = addPlacementConn("b");
  CQConnection *io1 = addPlacementConn("c");
  CQConnection *io2 = addPlacementConn("d");
  CQConnection *out = addPlacementConn("e");

  notGate1->connect("a", in1);
  notGate1->connect("c", io1);

  notGate2->connect("a", in2);
  notGate2->connect("c", io2);

  nandGate->connect("a", io1);
  nandGate->connect("b", io2);
  nandGate->connect("c", out);
}

void
CQSchem::
buildOr8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 7, 7);

  placementGroup->setCollapseName("or8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQOrGate *orGate[7];

  for (int i = 0; i < 7; ++i) {
    orGate[i] = addGateT<CQOrGate>();

    placementGroup->addGate(orGate[i], 6 - i, i);
  }

  CQConnection *in[8];

  for (int i = 0; i < 8; ++i)
    in[i] = addPlacementConn(QString("a%1").arg(i));

  CQConnection *out[7];

  for (int i = 0; i < 7; ++i)
    out[i] = addPlacementConn(QString("t%1").arg(i));

  for (int i = 0; i < 7; ++i) {
    if (i == 0)
      orGate[i]->connect("a", in [i]);
    else
      orGate[i]->connect("a", out[i - 1]);

    orGate[i]->connect("b", in [i + 1]);
    orGate[i]->connect("c", out[i    ]);
  }
}

void
CQSchem::
buildXorGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("xor");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNotGate  *notGate1  = addGateT<CQNotGate >("not1");
  CQNotGate  *notGate2  = addGateT<CQNotGate >("not2");
  CQNandGate *nandGate1 = addGateT<CQNandGate>("nand1");
  CQNandGate *nandGate2 = addGateT<CQNandGate>("nand2");
  CQNandGate *nandGate3 = addGateT<CQNandGate>("nand3");

  placementGroup->addGate(notGate1 , 1, 0);
  placementGroup->addGate(notGate2 , 0, 0);
  placementGroup->addGate(nandGate1, 1, 1);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 0, 2, 2, 1);

  CQConnection *acon = addPlacementConn("a");
  CQConnection *bcon = addPlacementConn("b");
  CQConnection *ccon = addPlacementConn("c");
  CQConnection *dcon = addPlacementConn("d");
  CQConnection *econ = addPlacementConn("e");
  CQConnection *fcon = addPlacementConn("f");
  CQConnection *gcon = addPlacementConn("g");

  notGate1->connect("a", acon);
  notGate1->connect("c", ccon);

  notGate2->connect("a", bcon);
  notGate2->connect("c", dcon);

  nandGate1->connect("a", ccon);
  nandGate1->connect("b", bcon);
  nandGate1->connect("c", econ);

  nandGate2->connect("a", acon);
  nandGate2->connect("b", dcon);
  nandGate2->connect("c", fcon);

  nandGate3->connect("a", econ);
  nandGate3->connect("b", fcon);
  nandGate3->connect("c", gcon);
}

void
CQSchem::
buildMemoryGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

  placementGroup->setCollapseName("memory");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQNandGate *nandGate1 = addGateT<CQNandGate>("1");
  CQNandGate *nandGate2 = addGateT<CQNandGate>("2");
  CQNandGate *nandGate3 = addGateT<CQNandGate>("3");
  CQNandGate *nandGate4 = addGateT<CQNandGate>("4");

  placementGroup->addGate(nandGate1, 1, 0);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 1, 2);
  placementGroup->addGate(nandGate4, 0, 2);

  CQConnection *coni = addPlacementConn("i");
  CQConnection *cons = addPlacementConn("s");
  CQConnection *cona = addPlacementConn("a");
  CQConnection *conb = addPlacementConn("b");
  CQConnection *conc = addPlacementConn("c");
  CQConnection *cono = addPlacementConn("o");

  nandGate1->connect("a", coni);
  nandGate1->connect("b", cons);
  nandGate1->connect("c", cona);

  nandGate2->connect("a", cona);
  nandGate2->connect("b", cons);
  nandGate2->connect("c", conb);

  nandGate3->connect("a", cona);
  nandGate3->connect("b", conc);
  nandGate3->connect("c", cono);

  nandGate4->connect("a", cono);
  nandGate4->connect("b", conb);
  nandGate4->connect("c", conc);
}

void
CQSchem::
buildMemory8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("memory8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  CQConnection *cons = addPlacementConn("s");

  CQMemoryGate *mem[8];

  for (int i = 0; i < 8; ++i) {
    QString memname = QString("mem%1").arg(i);

    mem[i] = addGateT<CQMemoryGate>(memname);

    mem[i]->setSSide(CQPort::Side::BOTTOM);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addPlacementConn(iname);
    CQConnection *cono = addPlacementConn(oname);

    mem[i]->connect("i", coni);
    mem[i]->connect("o", cono);
    mem[i]->connect("s", cons);

    ibus->addConnection(coni, i);
    obus->addConnection(cono, i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(mem[7 - i]);
}

void
CQSchem::
buildEnablerGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("enabler");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *ibus = addPlacementBus("i", 8);
  CQBus *obus = addPlacementBus("o", 8);

  CQConnection *cone = addPlacementConn("e");

  CQAndGate *gate[8];

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    gate[i] = addGateT<CQAndGate>(andname);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addPlacementConn(iname);
    CQConnection *cono = addPlacementConn(oname);

    gate[i]->connect("a", coni);
    gate[i]->connect("b", cone);
    gate[i]->connect("c", cono);

    ibus->addConnection(coni, i);
    obus->addConnection(cono, i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gate[7 - i]);
}

void
CQSchem::
buildRegisterGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQMemory8Gate *mem = addGateT<CQMemory8Gate>("B");
  CQEnablerGate *ena = addGateT<CQEnablerGate>("E");

  placementGroup->addGate(mem);
  placementGroup->addGate(ena);

  CQConnection *cons = addPlacementConn("s");
  CQConnection *cone = addPlacementConn("e");

  mem->connect("s", cons);
  ena->connect("e", cone);

  CQBus *ibus  = addPlacementBus( "i", 8);
  CQBus *iobus = addPlacementBus("io", 8);
  CQBus *obus  = addPlacementBus( "o", 8);

  for (int i = 0; i < 8; ++i) {
    QString ioname = QString("io%1").arg(i);

    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    CQConnection *icon  = addPlacementConn(iname);
    CQConnection *iocon = addPlacementConn(ioname);
    CQConnection *ocon  = addPlacementConn(oname);

    mem->connect(iname, icon);
    mem->connect(oname, iocon);

    ena->connect(iname, iocon);
    ena->connect(oname, ocon);

    ibus ->addConnection(icon , i);
    iobus->addConnection(iocon, i);
    obus ->addConnection(ocon , i);
  }
}

void
CQSchem::
buildDecoder4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 5, 3);

  placementGroup->setCollapseName("decoder4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQConnection *cona = addPlacementConn("a");
  CQConnection *conb = addPlacementConn("b");

  CQConnection *conna = addPlacementConn("na");
  CQConnection *connb = addPlacementConn("nb");

  CQNotGate *notgate[2];

  for (int i = 0; i < 2; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<CQNotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);

  CQAndGate *andgate[4];

  for (int i = 0; i < 4; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<CQAndGate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));

    QString oname = CQDecoder4Gate::oname(i);

    CQConnection *out = addPlacementConn(oname);

    andgate[i]->connect("c", out);
  }

  //---

  for (int i = 0; i < 2; ++i)
    placementGroup->addGate(notgate[i], 4 - i, 1 - i);

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(andgate[i], 3 - i, 2);
}

void
CQSchem::
buildDecoder8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 10, 4);

  placementGroup->setCollapseName("decoder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQConnection *cona = addPlacementConn("a");
  CQConnection *conb = addPlacementConn("b");
  CQConnection *conc = addPlacementConn("c");

  CQConnection *conna = addPlacementConn("na");
  CQConnection *connb = addPlacementConn("nb");
  CQConnection *connc = addPlacementConn("nc");

  CQNotGate *notgate[3];

  for (int i = 0; i < 3; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<CQNotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);
  notgate[2]->connect("a", conc);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);
  notgate[2]->connect("c", connc);

  CQAnd3Gate *andgate[8];

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<CQAnd3Gate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);
    int i3 = (i & 4);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));
    andgate[i]->connect("c", (i3 ? conc : connc));

    QString oname = CQDecoder8Gate::oname(i);

    CQConnection *out = addPlacementConn(oname);

    andgate[i]->connect("d", out);
  }

  //---

  for (int i = 0; i < 3; ++i)
    placementGroup->addGate(notgate[i], 9 - i, 2 - i);

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(andgate[i], 8 - i, 3);
}

void
CQSchem::
buildDecoder16Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 18, 5);

  placementGroup->setCollapseName("decoder16");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQConnection *cona = addPlacementConn("a");
  CQConnection *conb = addPlacementConn("b");
  CQConnection *conc = addPlacementConn("c");
  CQConnection *cond = addPlacementConn("d");

  CQConnection *conna = addPlacementConn("na");
  CQConnection *connb = addPlacementConn("nb");
  CQConnection *connc = addPlacementConn("nc");
  CQConnection *connd = addPlacementConn("nd");

  CQNotGate *notgate[4];

  for (int i = 0; i < 4; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<CQNotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);
  notgate[2]->connect("a", conc);
  notgate[3]->connect("a", cond);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);
  notgate[2]->connect("c", connc);
  notgate[3]->connect("c", connd);

  CQAnd4Gate *andgate[16];

  for (int i = 0; i < 16; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<CQAnd4Gate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);
    int i3 = (i & 4);
    int i4 = (i & 8);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));
    andgate[i]->connect("c", (i3 ? conc : connc));
    andgate[i]->connect("d", (i4 ? cond : connd));

    QString outname = CQDecoder16Gate::oname(i);

    CQConnection *out = addPlacementConn(outname);

    andgate[i]->connect("e", out);
  }

  //---

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(notgate[i], 17 - i, 3 - i);

  for (int i = 0; i < 16; ++i)
    placementGroup->addGate(andgate[i], 16 - i, 4);
}

void
CQSchem::
buildDecoder256Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 259, 9);

  placementGroup->setCollapseName("decoder256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQConnection *cona = addPlacementConn("a");
  CQConnection *conb = addPlacementConn("b");
  CQConnection *conc = addPlacementConn("c");
  CQConnection *cond = addPlacementConn("d");
  CQConnection *cone = addPlacementConn("e");
  CQConnection *conf = addPlacementConn("f");
  CQConnection *cong = addPlacementConn("g");
  CQConnection *conh = addPlacementConn("h");

  CQConnection *conna = addPlacementConn("na");
  CQConnection *connb = addPlacementConn("nb");
  CQConnection *connc = addPlacementConn("nc");
  CQConnection *connd = addPlacementConn("nd");
  CQConnection *conne = addPlacementConn("ne");
  CQConnection *connf = addPlacementConn("nf");
  CQConnection *conng = addPlacementConn("ng");
  CQConnection *connh = addPlacementConn("nh");

  CQNotGate *notgate[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<CQNotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);
  notgate[2]->connect("a", conc);
  notgate[3]->connect("a", cond);
  notgate[4]->connect("a", cone);
  notgate[5]->connect("a", conf);
  notgate[6]->connect("a", cong);
  notgate[7]->connect("a", conh);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);
  notgate[2]->connect("c", connc);
  notgate[3]->connect("c", connd);
  notgate[4]->connect("c", conne);
  notgate[5]->connect("c", connf);
  notgate[6]->connect("c", conng);
  notgate[7]->connect("c", connh);

  CQAnd8Gate *andgate[256];

  for (int i = 0; i < 256; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<CQAnd8Gate>(andname);

    int i1 = (i &   1);
    int i2 = (i &   2);
    int i3 = (i &   4);
    int i4 = (i &   8);
    int i5 = (i &  16);
    int i6 = (i &  32);
    int i7 = (i &  64);
    int i8 = (i & 128);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));
    andgate[i]->connect("c", (i3 ? conc : connc));
    andgate[i]->connect("d", (i4 ? cond : connd));
    andgate[i]->connect("e", (i5 ? cone : conne));
    andgate[i]->connect("f", (i6 ? conf : connf));
    andgate[i]->connect("g", (i7 ? cong : conng));
    andgate[i]->connect("h", (i8 ? conh : connh));

    QString outname = CQDecoder256Gate::oname(i);

    CQConnection *out = addPlacementConn(outname);

    andgate[i]->connect("i", out);
  }

  //---

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(notgate[i], 257 - i, 7 - i);

  for (int i = 0; i < 256; ++i)
    placementGroup->addGate(andgate[i], 256 - i, 8);
}

void
CQSchem::
buildLShift()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("lshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQRegisterGate *gate1 = addGateT<CQRegisterGate>("R1");
  CQRegisterGate *gate2 = addGateT<CQRegisterGate>("R2");

  placementGroup->addGate(gate1);
  placementGroup->addGate(gate2);

  CQConnection *iocon[9];

  iocon[0] = addPlacementConn("shift_out");
  iocon[8] = addPlacementConn("shift_in");

  for (int i = 1; i <= 7; ++i)
    iocon[i] = addPlacementConn(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *icon1 = addPlacementConn(iname);
    CQConnection *ocon2 = addPlacementConn(oname);

    gate1->connect(iname, icon1);
    gate1->connect(oname, iocon[i]);

    gate2->connect(iname, iocon[i + 1]);
    gate2->connect(oname, ocon2);
  }

  //---

  gate1->connect("s", addPlacementConn("s1"));
  gate1->connect("e", addPlacementConn("e1"));

  gate2->connect("s", addPlacementConn("s2"));
  gate2->connect("e", addPlacementConn("e2"));
}

void
CQSchem::
buildRShift()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("rshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQRegisterGate *gate1 = addGateT<CQRegisterGate>("R1");
  CQRegisterGate *gate2 = addGateT<CQRegisterGate>("R2");

  placementGroup->addGate(gate1);
  placementGroup->addGate(gate2);

  CQConnection *iocon[9];

  iocon[0] = addPlacementConn("shift_out");
  iocon[8] = addPlacementConn("shift_in");

  for (int i = 0; i < 7; ++i)
    iocon[i + 1] = addPlacementConn(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *icon1 = addPlacementConn(iname);
    CQConnection *ocon2 = addPlacementConn(oname);

    gate1->connect(iname, icon1);
    gate1->connect(oname, iocon[i]);

    gate2->connect(iname, iocon[i + 1]);
    gate2->connect(oname, ocon2);
  }

  //---

  gate1->connect("s", addPlacementConn("s1"));
  gate1->connect("e", addPlacementConn("e1"));

  gate2->connect("s", addPlacementConn("s2"));
  gate2->connect("e", addPlacementConn("e2"));
}

void
CQSchem::
buildInverter()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("inverter");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *ibus = addPlacementBus("a", 8);
  CQBus *obus = addPlacementBus("c", 8);

  CQNotGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("not%1").arg(i);

    gates[i] = addGateT<CQNotGate>(name);

    QString iname = QString("a%1").arg(i);
    QString oname = QString("c%1").arg(i);

    CQConnection *icon = addPlacementConn(iname);
    CQConnection *ocon = addPlacementConn(oname);

    gates[i]->connect("a", icon);
    gates[i]->connect("c", ocon);

    ibus->addConnection(icon, i);
    obus->addConnection(ocon, i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gates[7 - i]);
}

void
CQSchem::
buildAnder()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("ander");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *abus = addPlacementBus("a", 8); abus->setPosition(CQBus::Position::START,  0.25);
  CQBus *bbus = addPlacementBus("b", 8); bbus->setPosition(CQBus::Position::END  , -0.25);
  CQBus *cbus = addPlacementBus("c", 8);

  CQAndGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("and%1").arg(i);

    gates[i] = addGateT<CQAndGate>(name);

    QString aname = QString("a%1").arg(i);
    QString bname = QString("b%1").arg(i);
    QString cname = QString("c%1").arg(i);

    CQConnection *acon = addPlacementConn(aname);
    CQConnection *bcon = addPlacementConn(bname);
    CQConnection *ccon = addPlacementConn(cname);

    gates[i]->connect("a", acon);
    gates[i]->connect("b", bcon);
    gates[i]->connect("c", ccon);

    abus->addConnection(acon, i);
    bbus->addConnection(bcon, i);
    cbus->addConnection(ccon, i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gates[7 - i]);
}

void
CQSchem::
buildOrer()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("orer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *ibus1 = addPlacementBus("i1", 8); ibus1->setPosition(CQBus::Position::START,  0.25);
  CQBus *ibus2 = addPlacementBus("i2", 8); ibus2->setPosition(CQBus::Position::END  , -0.25);
  CQBus *obus  = addPlacementBus("o" , 8);

  CQOrGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("or%1").arg(i);

    gates[i] = addGateT<CQOrGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    CQConnection *icon1 = addPlacementConn(iname1);
    CQConnection *icon2 = addPlacementConn(iname2);
    CQConnection *ocon  = addPlacementConn(oname);

    gates[i]->connect("a", icon1);
    gates[i]->connect("b", icon2);
    gates[i]->connect("c", ocon );

    ibus1->addConnection(icon1, i);
    ibus2->addConnection(icon2, i);
    obus ->addConnection(ocon , i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gates[7 - i]);
}

void
CQSchem::
buildXorer()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("xorer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *ibus1 = addPlacementBus("i1", 8); ibus1->setPosition(CQBus::Position::START,  0.25);
  CQBus *ibus2 = addPlacementBus("i2", 8); ibus2->setPosition(CQBus::Position::END  , -0.25);
  CQBus *obus  = addPlacementBus("o" , 8);

  CQXorGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("xor%1").arg(i);

    gates[i] = addGateT<CQXorGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    CQConnection *icon1 = addPlacementConn(iname1);
    CQConnection *icon2 = addPlacementConn(iname2);
    CQConnection *ocon  = addPlacementConn(oname);

    gates[i]->connect("a", icon1);
    gates[i]->connect("b", icon2);
    gates[i]->connect("c", ocon );

    ibus1->addConnection(icon1, i);
    ibus2->addConnection(icon2, i);
    obus ->addConnection(ocon , i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gates[7 - i]);
}

void
CQSchem::
buildAdder()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("adder");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQXorGate *xorGate1 = addGateT<CQXorGate>("xor1");
  CQXorGate *xorGate2 = addGateT<CQXorGate>("xor2");
  CQAndGate *andGate1 = addGateT<CQAndGate>("and1");
  CQAndGate *andGate2 = addGateT<CQAndGate>("and2");
  CQOrGate  *orGate   = addGateT<CQOrGate >("or"  );

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(xorGate2, 2, 2);
  placementGroup->addGate(andGate1, 1, 1);
  placementGroup->addGate(andGate2, 0, 1);
  placementGroup->addGate(orGate  , 0, 2, 2, 1);

  CQConnection *acon  = addPlacementConn("a");
  CQConnection *bcon  = addPlacementConn("b");
  CQConnection *cconi = addPlacementConn("carry_in");
  CQConnection *scon  = addPlacementConn("sum");
  CQConnection *ccono = addPlacementConn("carry_out");

  CQConnection *scon1 = addPlacementConn("");
  CQConnection *ccon1 = addPlacementConn("");
  CQConnection *ccon2 = addPlacementConn("");

  xorGate1->connect("a", acon);
  xorGate1->connect("b", bcon);
  xorGate1->connect("c", scon1);

  xorGate2->connect("a", scon1);
  xorGate2->connect("b", cconi);
  xorGate2->connect("c", scon);

  andGate1->connect("a", cconi);
  andGate1->connect("b", scon1);
  andGate1->connect("c", ccon1);

  andGate2->connect("a", acon);
  andGate2->connect("b", bcon);
  andGate2->connect("c", ccon2);

  orGate->connect("a", ccon1);
  orGate->connect("b", ccon2);
  orGate->connect("c", ccono);
}

void
CQSchem::
buildAdder8()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("adder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQBus *abus = addPlacementBus("i"  , 8); abus->setPosition(CQBus::Position::START,  0.25);
  CQBus *bbus = addPlacementBus("o"  , 8); bbus->setPosition(CQBus::Position::END  , -0.25);
  CQBus *sbus = addPlacementBus("sum", 8);

  CQAdderGate  *adders[8];
  CQConnection *acon  [8];
  CQConnection *bcon  [8];
  CQConnection *scon  [8];

  CQConnection *cicon = nullptr;
  CQConnection *cocon = nullptr;

  for (int i = 0; i < 8; ++i) {
    adders[i] = addGateT<CQAdderGate>(QString("adder%1").arg(i));

    acon[i] = addPlacementConn(QString("a%1"  ).arg(i));
    bcon[i] = addPlacementConn(QString("b%1"  ).arg(i));
    scon[i] = addPlacementConn(QString("sum%1").arg(i));

    adders[i]->connect("a", acon[i]);
    adders[i]->connect("b", bcon[i]);

    if (! cicon)
      cicon = addPlacementConn("carry_in");

    cocon = addPlacementConn("carry_out");

    adders[i]->connect("carry_in" , cicon);
    adders[i]->connect("carry_out", cocon);

    cicon = cocon;

    adders[i]->connect("sum", scon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    sbus->addConnection(scon[i], i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(adders[7 - i]);
}

void
CQSchem::
buildComparator()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 4, 5);

  placementGroup->setCollapseName("comparator");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  CQXorGate  *xorGate1 = addGateT<CQXorGate >("xor1");
  CQNotGate  *notGate2 = addGateT<CQNotGate >("not2");
  CQAndGate  *andGate3 = addGateT<CQAndGate >("and3");
  CQAnd3Gate *andGate4 = addGateT<CQAnd3Gate>("and4");
  CQOrGate   *orGate5  = addGateT<CQOrGate  >("or5" );

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(notGate2, 1, 1);
  placementGroup->addGate(andGate3, 0, 2);
  placementGroup->addGate(andGate4, 3, 3);
  placementGroup->addGate(orGate5 , 0, 4);

  CQConnection *acon = addPlacementConn("a");
  CQConnection *bcon = addPlacementConn("b");
  CQConnection *ccon = addPlacementConn("c");

  CQConnection *equalCon     = addPlacementConn("equal");
  CQConnection *iallEqualCon = addPlacementConn("i_all_equal");
  CQConnection *oallEqualCon = addPlacementConn("o_all_equal");

  CQConnection *dcon = addPlacementConn("d");

  CQConnection *iaLargerCon = addPlacementConn("i_a_larger");
  CQConnection *oaLargerCon = addPlacementConn("o_a_larger");

  xorGate1->connect("a", acon);
  xorGate1->connect("b", bcon);
  xorGate1->connect("c", ccon);

  notGate2->connect("a", ccon);
  notGate2->connect("c", equalCon);

  andGate3->connect("a", iallEqualCon);
  andGate3->connect("b", equalCon);
  andGate3->connect("c", oallEqualCon);

  andGate4->connect("a", iallEqualCon);
  andGate4->connect("b", acon);
  andGate4->connect("c", ccon);
  andGate4->connect("d", dcon);

  orGate5->connect("a", iaLargerCon);
  orGate5->connect("b", dcon);
  orGate5->connect("c", oaLargerCon);

  andGate3->setOrientation(CQGate::Orientation::R90);
  orGate5 ->setOrientation(CQGate::Orientation::R90);
}

void
CQSchem::
buildComparator8()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("comparator8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQConnection *iallEqualCon = addPlacementConn("i_all_equal");
  CQConnection *iaLargerCon  = addPlacementConn("i_a_larger" );

  iallEqualCon->setValue(1);

  //---

  CQBus *abus = addPlacementBus("a", 8); abus->setPosition(CQBus::Position::START,  0.25);
  CQBus *bbus = addPlacementBus("b", 8); bbus->setPosition(CQBus::Position::END  , -0.25);
  CQBus *cbus = addPlacementBus("c", 8);

  //---

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];

  CQPlacementGroup *placementGroup1[8];

  for (int i = 0; i < 8; ++i) {
    placementGroup1[i] = new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 4, 5);

    //---

    CQXorGate  *xorGate1 = addGateT<CQXorGate >(QString("xor1[%1]").arg(i));
    CQNotGate  *notGate2 = addGateT<CQNotGate >(QString("not2[%1]").arg(i));
    CQAndGate  *andGate3 = addGateT<CQAndGate >(QString("and3[%1]").arg(i));
    CQAnd3Gate *andGate4 = addGateT<CQAnd3Gate>(QString("and4[%1]").arg(i));
    CQOrGate   *orGate5  = addGateT<CQOrGate  >(QString("or5[%1]" ).arg(i));

    placementGroup1[i]->addGate(xorGate1, 2, 0);
    placementGroup1[i]->addGate(notGate2, 1, 1);
    placementGroup1[i]->addGate(andGate3, 0, 2);
    placementGroup1[i]->addGate(andGate4, 3, 3);
    placementGroup1[i]->addGate(orGate5 , 0, 4);

    //---

    acon[i] = addPlacementConn("a");
    bcon[i] = addPlacementConn("b");
    ccon[i] = addPlacementConn("c");

    CQConnection *equalCon = addPlacementConn("equal");

    CQConnection *dcon = addPlacementConn("d");

    CQConnection *oallEqualCon = addPlacementConn("o_all_equal");
    CQConnection *oaLargerCon  = addPlacementConn("o_a_larger" );

    xorGate1->connect("a", acon[i]);
    xorGate1->connect("b", bcon[i]);
    xorGate1->connect("c", ccon[i]);

    notGate2->connect("a", ccon[i]);
    notGate2->connect("c", equalCon);

    andGate3->connect("a", iallEqualCon);
    andGate3->connect("b", equalCon);
    andGate3->connect("c", oallEqualCon);

    andGate4->connect("a", iallEqualCon);
    andGate4->connect("b", acon[i]);
    andGate4->connect("c", ccon[i]);
    andGate4->connect("d", dcon);

    orGate5->connect("a", iaLargerCon);
    orGate5->connect("b", dcon);
    orGate5->connect("c", oaLargerCon);

    andGate3->setOrientation(CQGate::Orientation::R90);
    orGate5 ->setOrientation(CQGate::Orientation::R90);

    //---

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);

    iallEqualCon = oallEqualCon;
    iaLargerCon  = oaLargerCon;
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addPlacementGroup(placementGroup1[7 - i]);
}

void
CQSchem::
buildBus0()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("bus0");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQOr8Gate *orGate  = addGateT<CQOr8Gate>("or");
  CQNotGate *notGate = addGateT<CQNotGate>("not");

  CQBus *ibus = addPlacementBus("ibus", 8);

  ibus->setGate(orGate);

  CQConnection *icon[8];

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(CQOr8Gate::iname(i));

    orGate->connect(CQOr8Gate::iname(i), icon[i]);

    ibus->addConnection(icon[i], i);
  }

  CQConnection *ocon = addPlacementConn("o");
  CQConnection *zcon = addPlacementConn("zero");

  orGate->connect("o", ocon);

  notGate->connect("a", ocon);
  notGate->connect("c", zcon);
}

void
CQSchem::
buildBus1()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 8);

  placementGroup->setCollapseName("bus1");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQAndGate *andGate[7];

  for (int i = 0; i < 7; ++i) {
    andGate[i] = addGateT<CQAndGate>(QString("and%1").arg(i));

    andGate[i]->setOrientation(CQGate::Orientation::R90);
  }

  CQOrGate *orGate = addGateT<CQOrGate >("or");

  orGate->setOrientation(CQGate::Orientation::R90);

  CQNotGate *notGate = addGateT<CQNotGate>("not");

  notGate->setOrientation(CQGate::Orientation::R180);

  CQBus *ibus = addPlacementBus("ibus", 8);
  CQBus *obus = addPlacementBus("obus", 8);

  ibus->setFlipped(true);
  obus->setFlipped(true);

  CQConnection *bcon1 = addPlacementConn("bus1");
  CQConnection *bcon2 = addPlacementConn("nbus1");

  CQConnection *icon[8];
  CQConnection *ocon[8];

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(QString("i%1").arg(i));
    ocon[i] = addPlacementConn(QString("o%1").arg(i));

    if (i < 7) {
      andGate[i]->connect("a", bcon2);
      andGate[i]->connect("b", icon[i]);
      andGate[i]->connect("c", ocon[i]);
    }
    else {
      orGate->connect("a", bcon1);
      orGate->connect("b", icon[i]);
      orGate->connect("c", ocon[i]);
    }

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }

  notGate->connect("a", bcon1);
  notGate->connect("c", bcon2);

  for (int i = 0; i < 7; ++i)
    placementGroup->addGate(andGate[i], 0, i);

  placementGroup->addGate(orGate , 0, 7);
  placementGroup->addGate(notGate, 1, 7);
}

void
CQSchem::
buildRam256()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

  placementGroup->setCollapseName("ram256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQRegisterGate *rgate = addGateT<CQRegisterGate>("R");

  placementGroup->addGate(rgate, 1, 0);

  //--

  CQDecoder16Gate *hdec = addGateT<CQDecoder16Gate>("H 4x16");
  CQDecoder16Gate *vdec = addGateT<CQDecoder16Gate>("V 4x16");

  placementGroup->addGate(hdec, 1, 2, 1, 1, CQPlacementGroup::Alignment::HFILL);
  placementGroup->addGate(vdec, 0, 1, 1, 1, CQPlacementGroup::Alignment::VFILL);

  hdec->setOrientation(CQGate::Orientation::R90);

  //---

  CQBus *ibus  = addPlacementBus("i" , 8);
  CQBus *obus1 = addPlacementBus("o1", 4);
  CQBus *obus2 = addPlacementBus("o2", 4);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *in  = addPlacementConn(iname);
    CQConnection *out = addPlacementConn(oname);

    rgate->connect(iname, in );
    rgate->connect(oname, out);

    if (i < 4)
      hdec->connect(CQDecoder16Gate::iname(i    ), out);
    else
      vdec->connect(CQDecoder16Gate::iname(i - 4), out);

    ibus->addConnection(in , i);

    if (i < 4)
      obus1->addConnection(out, i);
    else
      obus2->addConnection(out, i - 4);
  }

  CQConnection *hout[16], *vout[16];

  for (int i = 0; i < 16; ++i) {
    QString honame = CQDecoder16Gate::oname(i);
    QString voname = CQDecoder16Gate::oname(i);

    hout[i] = addPlacementConn(honame);
    vout[i] = addPlacementConn(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate->connect("s", addPlacementConn("sa"));

  CQConnection *s = addPlacementConn("s");
  CQConnection *e = addPlacementConn("e");

  //---

  CQConnection *bus[8];

  CQBus *iobus = addPlacementBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addPlacementConn(QString("bus[%1]").arg(i));

    iobus->addConnection(bus[i], i);
  }

  //---

  CQPlacementGroup *placementGroup1 =
    new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 16, 16);

  for (int r = 0; r < 16; ++r) {
    for (int c = 0; c < 16; ++c) {
      CQPlacementGroup *placementGroup2 =
        new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

      //---

      CQAndGate *xgate = addGateT<CQAndGate>(QString("X_%1_%2").arg(r).arg(c));

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      CQConnection *t1 = addPlacementConn("t1");

      xgate->connect("c", t1);

      CQAndGate *agate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));
      CQAndGate *bgate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));

      CQRegisterGate *rgate = addGateT<CQRegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      CQConnection *t2 = addPlacementConn("t2");
      CQConnection *t3 = addPlacementConn("t3");

      agate->connect("a", t1);
      agate->connect("b", s);
      agate->connect("c", t2);

      bgate->connect("a", t1);
      bgate->connect("b", e);
      bgate->connect("c", t3);

      rgate->connect("s", t2);
      rgate->connect("e", t3);

      for (int i = 0; i < 8; ++i) {
        QString iname = CQRegisterGate::iname(i);
        QString oname = CQRegisterGate::oname(i);

        rgate->connect(iname, bus[i]);
        rgate->connect(oname, bus[i]);
      }

      //---

      placementGroup2->addGate(xgate, 1, 0);
      placementGroup2->addGate(agate, 1, 1);
      placementGroup2->addGate(bgate, 0, 1);
      placementGroup2->addGate(rgate, 0, 2, 2, 1);

      //---

      placementGroup1->addPlacementGroup(placementGroup2, 15 - r, c);
    }
  }

  placementGroup->addPlacementGroup(placementGroup1, 0, 2);
}

void
CQSchem::
buildRam65536()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("ram65536");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQRegisterGate *rgate0 = addGateT<CQRegisterGate>("R0");
  CQRegisterGate *rgate1 = addGateT<CQRegisterGate>("R1");

  placementGroup->addGate(rgate0, 0, 0);
  placementGroup->addGate(rgate1, 2, 2);

  rgate1->setOrientation(CQGate::Orientation::R90);

  //--

  CQDecoder256Gate *hdec = addGateT<CQDecoder256Gate>("H 8x256");
  CQDecoder256Gate *vdec = addGateT<CQDecoder256Gate>("V 8x256");

  placementGroup->addGate(hdec, 1, 2, 1, 1, CQPlacementGroup::Alignment::HFILL);
  placementGroup->addGate(vdec, 0, 1, 1, 1, CQPlacementGroup::Alignment::VFILL);

  hdec->setOrientation(CQGate::Orientation::R90);

  //---

  CQBus *ibus  = addPlacementBus("i" , 8);
  CQBus *obus1 = addPlacementBus("o1", 8);
  CQBus *obus2 = addPlacementBus("o2", 8);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *in   = addPlacementConn(iname);
    CQConnection *out1 = addPlacementConn(oname);
    CQConnection *out2 = addPlacementConn(oname);

    rgate0->connect(iname, in);
    rgate1->connect(iname, in);

    rgate0->connect(oname, out1);
    rgate1->connect(oname, out2);

    if (i < 4)
      hdec->connect(CQDecoder256Gate::iname(i), out1);
    else
      vdec->connect(CQDecoder256Gate::iname(i), out2);

    ibus->addConnection(in, i);

    obus1->addConnection(out1, i);
    obus2->addConnection(out2, i);
  }

  CQConnection *hout[256], *vout[256];

  for (int i = 0; i < 256; ++i) {
    QString honame = CQDecoder256Gate::oname(i);
    QString voname = CQDecoder256Gate::oname(i);

    hout[i] = addPlacementConn(honame);
    vout[i] = addPlacementConn(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate0->connect("s", addPlacementConn("s0"));
  rgate1->connect("s", addPlacementConn("s1"));

  CQConnection *s = addPlacementConn("s");
  CQConnection *e = addPlacementConn("e");

  //---

  CQConnection *bus[8];

  CQBus *iobus = addPlacementBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addPlacementConn(QString("bus[%1]").arg(i));

    iobus->addConnection(bus[i], i);
  }

  //---

  CQPlacementGroup *placementGroup1 =
    new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 256, 256);

  for (int r = 0; r < 256; ++r) {
    //std::cerr << "Row: " << r << "\n";

    for (int c = 0; c < 256; ++c) {
      CQPlacementGroup *placementGroup2 =
        new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

      //---

      CQAndGate *xgate = addGateT<CQAndGate>(QString("X_%1_%2").arg(r).arg(c));

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      CQConnection *t1 = addPlacementConn("t1");

      xgate->connect("c", t1);

      CQAndGate *agate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));
      CQAndGate *bgate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));

      CQRegisterGate *rgate = addGateT<CQRegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      CQConnection *t2 = addPlacementConn("t2");
      CQConnection *t3 = addPlacementConn("t3");

      agate->connect("a", t1);
      agate->connect("b", s);
      agate->connect("c", t2);

      bgate->connect("a", t1);
      bgate->connect("b", e);
      bgate->connect("c", t3);

      rgate->connect("s", t2);
      rgate->connect("e", t3);

      for (int i = 0; i < 8; ++i) {
        QString iname = CQRegisterGate::iname(i);
        QString oname = CQRegisterGate::oname(i);

        rgate->connect(iname, bus[i]);
        rgate->connect(oname, bus[i]);
      }

      //---

      placementGroup2->addGate(xgate, 0, 0);
      placementGroup2->addGate(agate, 0, 1);
      placementGroup2->addGate(bgate, 1, 1);
      placementGroup2->addGate(rgate, 1, 2, 2, 1);

      //---

      placementGroup1->addPlacementGroup(placementGroup2, 255 - r, c);
    }
  }

  placementGroup->addPlacementGroup(placementGroup1, 0, 2);
}

void
CQSchem::
buildAlu()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 8, 4);

  placementGroup->setCollapseName("alu");

  //---

  auto addPlacementConn = [&](const QString &name) {
    CQConnection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    CQBus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  CQComparator8Gate *xorerGate   = addGateT<CQComparator8Gate>("XOR");
  CQOrerGate        *orerGate    = addGateT<CQOrerGate       >("OR" );
  CQAnderGate       *anderGate   = addGateT<CQAnderGate      >("AND");
  CQInverterGate    *noterGate   = addGateT<CQInverterGate   >("NOT");
  CQLShiftGate      *lshiftGate  = addGateT<CQLShiftGate     >("SHL");
  CQRShiftGate      *rshiftGate  = addGateT<CQRShiftGate     >("SHR");
  CQAdder8Gate      *adderGate   = addGateT<CQAdder8Gate     >("ADD");
  CQDecoder8Gate    *decoderGate = addGateT<CQDecoder8Gate   >("3x8");

  CQGate *gates[7];

  gates[0] = xorerGate;
  gates[1] = orerGate;
  gates[2] = anderGate;
  gates[3] = noterGate;
  gates[4] = lshiftGate;
  gates[5] = rshiftGate;
  gates[6] = adderGate;

  CQEnablerGate *enablerGate[7];

  for (int i = 0; i < 7; ++i)
    enablerGate[i] = addGateT<CQEnablerGate>("E");

  CQAndGate *andGate[3];

  for (int i = 0; i < 3; ++i)
    andGate[i] = addGateT<CQAndGate>("and");

  CQBus0Gate *zeroGate = addGateT<CQBus0Gate>("Z");

  placementGroup->addGate(xorerGate  , 7, 0);
  placementGroup->addGate(orerGate   , 6, 0);
  placementGroup->addGate(anderGate  , 5, 0);
  placementGroup->addGate(noterGate  , 4, 0);
  placementGroup->addGate(lshiftGate , 3, 0);
  placementGroup->addGate(rshiftGate , 2, 0);
  placementGroup->addGate(adderGate  , 1, 0);
  placementGroup->addGate(decoderGate, 0, 1);

  for (int i = 0; i < 3; ++i) {
    CQPlacementGroup *placementGroup1 =
      new CQPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

    placementGroup1->addGate(enablerGate[i]);
    placementGroup1->addGate(andGate    [i]);

    placementGroup->addPlacementGroup(placementGroup1, i + 1, 2);
  }

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(enablerGate[3 + i], 4 + i, 2);

  placementGroup->addGate(zeroGate, 6, 3, 2, 1);

  //---

  CQBus *abus = addPlacementBus("a", 8);
  CQBus *bbus = addPlacementBus("b", 8);
  CQBus *cbus = addPlacementBus("c", 8);

  CQBus *cbus1[7];

  for (int i = 0; i < 7; ++i) {
    cbus1[i] = addPlacementBus("c", 8);

    cbus1[i]->setGate(gates[i]);
  }

  //---

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];
  CQConnection *ccon1[7][8];

  for (int i = 0; i < 8; ++i) {
    QString aname = QString("a%1").arg(i);
    QString bname = QString("b%1").arg(i);
    QString cname = QString("c%1").arg(i);

    acon[i] = addPlacementConn(aname);
    bcon[i] = addPlacementConn(bname);
    ccon[i] = addPlacementConn(cname);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);

    for (int j = 0; j < 7; ++j) {
      ccon1[j][i] = addPlacementConn(cname);

      cbus1[j]->addConnection(ccon1[j][i], i);
    }
  }

  for (int i = 0; i < 8; ++i) {
    QString aname = QString("a%1").arg(i);
    QString bname = QString("b%1").arg(i);
    QString cname = QString("c%1").arg(i);

    auto connectABC = [&](CQGate *gate, int ig) {
      gate->connect(aname, acon[i]);
      gate->connect(bname, bcon[i]);
      gate->connect(cname, ccon1[ig][i]);
    };

    connectABC(xorerGate, 0);
    connectABC(orerGate , 1);
    connectABC(anderGate, 2);

    noterGate->connect(aname, acon[i]);
    noterGate->connect(cname, ccon1[3][i]);

    lshiftGate->connect(CQLShiftGate::iname(i), acon[i]);
    lshiftGate->connect(CQLShiftGate::oname(i), ccon1[4][i]);

    rshiftGate->connect(CQRShiftGate::iname(i), acon[i]);
    rshiftGate->connect(CQRShiftGate::oname(i), ccon1[5][i]);

    connectABC(adderGate, 6);
  }

  for (int j = 0; j < 7; ++j) {
    for (int i = 0; i < 8; ++i) {
      enablerGate[j]->connect(CQEnablerGate::iname(i), ccon1[j][i]);
      enablerGate[j]->connect(CQEnablerGate::oname(i), ccon[i]);
    }
  }

  xorerGate->connect("a_larger", addPlacementConn("a_larger"));
  xorerGate->connect("equal"   , addPlacementConn("equal"   ));

  for (int i = 0; i < 8; ++i)
    zeroGate->connect(CQBus0Gate::iname(i), ccon[i]);

  zeroGate->connect("zero", addPlacementConn("zero"));

  decoderGate->connect(CQDecoder8Gate::iname(0), addPlacementConn("d1"));
  decoderGate->connect(CQDecoder8Gate::iname(1), addPlacementConn("d2"));
  decoderGate->connect(CQDecoder8Gate::iname(2), addPlacementConn("d3"));

  CQConnection *econ[8];

  for (int i = 0; i < 8; ++i) {
    econ[i] = addPlacementConn("e");

    decoderGate->connect(CQDecoder8Gate::oname(i), econ[i]);

    if (i > 0)
      enablerGate[i - 1]->connect("e", econ[i]);
  }

  CQConnection *carry_out1 = addPlacementConn("carry_out");

  CQConnection aocon[3];

  for (int i = 0; i < 3; ++i)
    andGate[i]->connect("c", carry_out1);

  CQConnection *lshift_out = addPlacementConn("shift_out");
  CQConnection *rshift_out = addPlacementConn("shift_out");
  CQConnection *carry_out  = addPlacementConn("carry_out");

  andGate[0]->connect("a", lshift_out);
  andGate[0]->connect("b", econ[5]);

  andGate[1]->connect("a", rshift_out);
  andGate[1]->connect("b", econ[6]);

  andGate[2]->connect("a", carry_out);
  andGate[2]->connect("b", econ[7]);
}

//------

CQConnection *
CQSchem::
addConnection(const QString &name)
{
  CQConnection *connection = new CQConnection(name);

  connections_.push_back(connection);

  return connection;
}

void
CQSchem::
removeConnection(CQConnection *connection)
{
  int i = 0;
  int n = connections_.size();

  for ( ; i < n; ++i) {
    if (connections_[i] == connection)
      break;
  }

  assert(i < n);

  delete connections_[i++];

  for ( ; i < n; ++i)
    connections_[i - 1] = connections_[i];

  connections_.pop_back();
}

void
CQSchem::
addGate(CQGate *gate)
{
  gates_.push_back(gate);

  placementGroup_->addGate(gate);
}

void
CQSchem::
removeGate(CQGate *gate)
{
  int i = 0;
  int n = gates_.size();

  for ( ; i < n; ++i) {
    if (gates_[i] == gate)
      break;
  }

  assert(i < n);

  delete gates_[i++];

  for ( ; i < n; ++i)
    gates_[i - 1] = gates_[i];

  gates_.pop_back();
}

CQBus *
CQSchem::
addBus(const QString &name, int n)
{
  CQBus *bus = new CQBus(name, n);

  buses_.push_back(bus);

  return bus;
}

void
CQSchem::
removeBus(CQBus *bus)
{
  int i = 0;
  int n = buses_.size();

  for ( ; i < n; ++i) {
    if (buses_[i] == bus)
      break;
  }

  assert(i < n);

  delete buses_[i++];

  for ( ; i < n; ++i)
    buses_[i - 1] = buses_[i];

  buses_.pop_back();
}

CQPlacementGroup *
CQSchem::
addPlacementGroup(CQPlacementGroup::Placement placement, int nr, int nc)
{
  CQPlacementGroup *placementGroup = new CQPlacementGroup(placement, nr, nc);

  addPlacementGroup(placementGroup);

  return placementGroup;
}

void
CQSchem::
addPlacementGroup(CQPlacementGroup *placementGroup)
{
  placementGroup_->addPlacementGroup(placementGroup);
}

void
CQSchem::
exec()
{
  bool changed = true;

  while (changed) {
    changed = false;

    for (auto &gate : gates_) {
      if (gate->exec())
        changed = true;
    }
  }
}

void
CQSchem::
test()
{
  std::vector<CQConnection *> in, out;

  for (const auto &connection : connections_) {
    if      (connection->isInput())
      in.push_back(connection);
    else if (connection->isOutput())
      out.push_back(connection);
  }

  int ni = in .size();
  int no = out.size();

  int n = std::pow(2, ni);

  for (int j = 0; j < ni; ++j)
    std::cerr << in[ni - j - 1]->name().toStdString();

  std::cerr << " = ";

  for (int j = 0; j < no; ++j)
    std::cerr << out[no - j - 1]->name().toStdString();

  std::cerr << "\n";

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < ni; ++j)
      in[j]->setValue(i & (1 << j));

    exec();

    //---

    for (int j = 0; j < ni; ++j)
      std::cerr << (in[ni - j - 1]->getValue() ? "1" : "0");

    std::cerr << " = ";

    for (int j = 0; j < no; ++j)
      std::cerr << (out[no - j - 1]->getValue() ? "1" : "0");

    std::cerr << "\n";
  }
}

void
CQSchem::
place()
{
  placementGroup_->place();

  calcBounds();
}

void
CQSchem::
calcBounds()
{
  rect_ = QRectF();

  for (auto &gate : gates_) {
    QRectF rect = gate->rect();

    if (! rect_.isValid())
      rect_ = rect;
    else
      rect_ = rect_.united(rect);
  }

  //---

  double a = (height() > 0 ? 1.0*width()/height() : 1.0);

  double w = rect_.width ();
  double h = rect_.height();

  if (a > 1.0)
    h *= a;
  else
    w *= a;

  double m = renderer_.pixelWidthToWindowWidth(100);

  QPointF c = rect_.center();

  rect_ = QRectF(c.x() - w/2 - m, c.y() - h/2 - m, w + 2*m, h + 2*m);

  //---

  renderer_.displayRange.setWindowRange(rect_.left(), rect_.top(), rect_.right(), rect_.bottom());
}

void
CQSchem::
resizeEvent(QResizeEvent *)
{
  renderer_.displayRange.setEqualScale(true);

  renderer_.displayRange.setPixelRange(0, 0, this->width() - 1, this->height() - 1);

  calcBounds();
}

void
CQSchem::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  painter.fillRect(rect(), QBrush(Qt::black));

  renderer_.schem         = this;
  renderer_.painter       = &painter;
  renderer_.prect         = rect();
  renderer_.rect          = rect_;
  renderer_.placementRect = placementGroup_->rect();

  for (const auto &gate : gates_)
    gate->draw(&renderer_);

  for (const auto &connection : connections_) {
    if (! connection->bus())
      connection->draw(&renderer_);
  }

  for (const auto &bus : buses_)
    bus->draw(&renderer_);

  placementGroup_->draw(&renderer_);
}

void
CQSchem::
mousePressEvent(QMouseEvent *e)
{
  pressed_    = false;
  pressPoint_ = QPointF(e->x(), e->y());

  if      (e->button() == Qt::LeftButton) {
    pressGate_      = nullptr;
    pressPlacement_ = nullptr;

    if      (moveGate_) {
      pressGate_ = nearestGate(pressPoint_);

      if (pressGate_) {
        pressGate_->setSelected(! pressGate_->isSelected());

        update();
      }
    }
    else if (movePlacement_) {
      pressPlacement_ = nearestPlacementGroup(pressPoint_);

      if (pressPlacement_) {
        pressPlacement_->setSelected(! pressPlacement_->isSelected());

        update();
      }
    }
    else if (moveConnection_) {
      pressConnection_ = nearestConnection(pressPoint_);

      if (pressConnection_) {
        pressConnection_->setSelected(! pressConnection_->isSelected());

        update();
      }
    }

    pressed_ = true;
  }
  else if (e->button() == Qt::MidButton) {
    pressConnection_ = nearestConnection(pressPoint_);

    if (pressConnection_) {
      if (pressConnection_->isInput()) {
        pressConnection_->setValue(! pressConnection_->getValue());

        exec();

        update();
      }
    }
  }
  else if (e->button() == Qt::RightButton) {
    QMenu *menu = new QMenu(this);

    auto addAction = [&](QMenu *menu, const QString &name, const char *slotName) {
      QAction *action = new QAction(name, menu);

      connect(action, SIGNAL(triggered()), this, slotName);

      menu->addAction(action);

      return action;
    };

    addAction(menu, "Expand"  , SLOT(expandSlot()));
    addAction(menu, "Collapse", SLOT(collapseSlot()));

    menu->exec(mapToGlobal(e->pos()));
  }
}

void
CQSchem::
mouseMoveEvent(QMouseEvent *e)
{
  movePoint_ = QPointF(e->x(), e->y());

  window_->setPos(renderer_.pixelToWindow(movePoint_));

  if (! pressed_) {
    if      (moveGate_) {
      CQGate *gate = nearestGate(movePoint_);

      if (gate != insideGate_) {
        insideGate_ = gate;

        update();
      }
    }
    else if (movePlacement_) {
      CQPlacementGroup *placement = nearestPlacementGroup(movePoint_);

      if (placement != insidePlacement_) {
        insidePlacement_ = placement;

        update();
      }
    }
    else if (moveConnection_) {
      CQConnection *connection = nearestConnection(movePoint_);

      if (connection != insideConnection_) {
        insideConnection_ = connection;

        update();
      }
    }

    return;
  }

  if      (pressGate_) {
    QPointF p1 = renderer_.pixelToWindow(QPointF(pressPoint_.x(), pressPoint_.y()));
    QPointF p2 = renderer_.pixelToWindow(QPointF(movePoint_ .x(), movePoint_ .y()));

    pressGate_->setRect(pressGate_->rect().translated(p2.x() - p1.x(), p2.y() - p1.y()));

    calcBounds();

    update();
  }
  else if (pressPlacement_) {
    QPointF p1 = renderer_.pixelToWindow(QPointF(pressPoint_.x(), pressPoint_.y()));
    QPointF p2 = renderer_.pixelToWindow(QPointF(movePoint_ .x(), movePoint_ .y()));

    pressPlacement_->setRect(pressPlacement_->rect().translated(p2.x() - p1.x(), p2.y() - p1.y()));

    calcBounds();

    update();
  }

  pressPoint_ = movePoint_;
}

void
CQSchem::
mouseReleaseEvent(QMouseEvent *e)
{
  mouseMoveEvent(e);

  pressed_ = false;
}

void
CQSchem::
keyPressEvent(QKeyEvent *e)
{
  if      (e->key() == Qt::Key_Plus)
    renderer_.displayTransform.zoomIn();
  else if (e->key() == Qt::Key_Minus)
    renderer_.displayTransform.zoomOut();
  else if (e->key() == Qt::Key_Left)
    renderer_.displayTransform.panLeft();
  else if (e->key() == Qt::Key_Right)
    renderer_.displayTransform.panRight();
  else if (e->key() == Qt::Key_Up)
    renderer_.displayTransform.panUp();
  else if (e->key() == Qt::Key_Down)
    renderer_.displayTransform.panDown();
  else if (e->key() == Qt::Key_Home)
    renderer_.displayTransform.reset();
  else if (e->key() == Qt::Key_Greater) {
    Gates gates;

    selectedGates(gates);

    for (auto &gate : gates)
      gate->nextOrient();
  }
  else if (e->key() == Qt::Key_Less) {
    Gates gates;

    selectedGates(gates);

    for (auto &gate : gates)
      gate->prevOrient();
  }
  else if (e->key() == Qt::Key_F) {
    Gates gates;

    selectedGates(gates);

    for (auto &gate : gates)
      gate->setFlipped(! gate->isFlipped());
  }

  update();
}

void
CQSchem::
expandSlot()
{
  std::vector<CQPlacementGroup *> expandGroups;

  for (auto &placementGroupData : placementGroup_->placementGroups()) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (placementGroup->expandName() != "")
      expandGroups.push_back(placementGroup);
  }

  for (auto &placementGroup : expandGroups) {
    CQPlacementGroup *parentGroup = placementGroup->parent();

    bool rc = execGate(placementGroup->expandName());
    assert(rc);

    parentGroup->replacePlacementGroup(this, placementGroup);
  }

  place();

  update();
}

void
CQSchem::
collapseSlot()
{
  std::vector<CQPlacementGroup *> collapseGroups;

  for (auto &placementGroupData : placementGroup_->placementGroups()) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (placementGroup->collapseName() != "")
      collapseGroups.push_back(placementGroup);
  }

  for (auto &placementGroup : collapseGroups) {
    CQPlacementGroup *parentGroup = placementGroup->parent();

    bool rc = execGate(placementGroup->collapseName());
    assert(rc);

    parentGroup->replacePlacementGroup(this, placementGroup);
  }

  place();

  update();
}

void
CQSchem::
selectedGates(Gates &gates) const
{
  for (auto &gate : gates_) {
    if (gate->isSelected())
      gates.push_back(gate);
  }
}

CQGate *
CQSchem::
nearestGate(const QPointF &p) const
{
  for (auto &gate : gates_) {
    if (gate->inside(p))
      return gate;
  }

  return nullptr;
}

CQPlacementGroup *
CQSchem::
nearestPlacementGroup(const QPointF &p) const
{
  for (auto &placementGroupData : placementGroup_->placementGroups()) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (placementGroup->inside(p))
      return placementGroup;
  }

  if (placementGroup_->inside(p))
    return placementGroup_;

  return nullptr;
}

CQConnection *
CQSchem::
nearestConnection(const QPointF &p) const
{
  for (auto &gate : gates_) {
    for (auto &port : gate->inputs()) {
      if (port->connection() && port->connection()->inside(p))
        return port->connection();
    }

    for (auto &port : gate->outputs()) {
      if (port->connection() && port->connection()->inside(p))
        return port->connection();
    }
  }

  return nullptr;
}

void
CQSchem::
drawConnection(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2)
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  if (dx > dy) {
    if (dy > 1E-6) {
      double ym = (p1.y() + p2.y())/2.0;

      CQSchem::drawLine(renderer, p1                 , QPointF(p1.x(), ym));
      CQSchem::drawLine(renderer, QPointF(p1.x(), ym), QPointF(p2.x(), ym));
      CQSchem::drawLine(renderer, QPointF(p2.x(), ym), p2                 );
    }
    else
      CQSchem::drawLine(renderer, p1, p2);
  }
  else {
    if (dx > 1E-6) {
      double xm = (p1.x() + p2.x())/2.0;

      CQSchem::drawLine(renderer, p1                 , QPointF(xm, p1.y()));
      CQSchem::drawLine(renderer, QPointF(xm, p1.y()), QPointF(xm, p2.y()));
      CQSchem::drawLine(renderer, QPointF(xm, p2.y()), p2                 );
    }
    else
      CQSchem::drawLine(renderer, p1, p2);
  }
}

void
CQSchem::
drawTextInRect(CQSchemRenderer *renderer, const QRectF &r, const QString &text)
{
  if (r.height() < 3)
    return;

  renderer->setFontSize(r.height());

  QPointF c = r.center();

  drawTextAtPoint(renderer, c, text);
}

void
CQSchem::
drawTextOnLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2,
               const QString &name, TextLinePos pos)
{
  if      (pos == TextLinePos::START) {
    QPointF pt(std::min(p1.x(), p2.x()) - 4, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name, CQSchem::TextAlign::RIGHT);
  }
  else if (pos == TextLinePos::END) {
    QPointF pt(std::max(p1.x(), p2.x()) + 4, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name, CQSchem::TextAlign::LEFT);
  }
  else {
    QPointF pt((p1.x() + p2.x())/2, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name);
  }
}

void
CQSchem::
drawTextAtPoint(CQSchemRenderer *renderer, const QPointF &p, const QString &text,
                TextAlign align)
{
  double fh = renderer->windowHeightToPixelHeight(0.25);
  if (fh < 3) return;

  renderer->setFontSize(fh);

  QFontMetricsF fm(renderer->painter->font());

  double dx = fm.width(text);
  double dy = (fm.ascent() - fm.descent())/2.0;

  QPointF pm;

  if      (align == TextAlign::CENTER)
    pm = QPointF(p.x() - dx/2.0, p.y() + dy);
  else if (align == TextAlign::RIGHT)
    pm = QPointF(p.x() - dx, p.y() + dy);
  else
    pm = QPointF(p.x(), p.y() + dy);

  renderer->painter->drawText(pm, text);
}

void
CQSchem::
drawLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2)
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  assert(int(dx) <= 1 || int(dy) <= 1);

  renderer->painter->drawLine(p1, p2);
}

//---

CQGate::
CQGate(const QString &name) :
 name_(name)
{
}

CQGate::
~CQGate()
{
  for (auto &port : inputs_)
    delete port;

  for (auto &port : outputs_)
    delete port;
}

QSizeF
CQGate::
calcSize() const
{
  return QSizeF(width(), height());
}

void
CQGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  if (renderer->schem->isShowGateText()) {
    renderer->painter->setPen(renderer->textColor);

    CQSchem::drawTextInRect(renderer, prect_, name());
  }

//renderer->painter->setPen(Qt::red);
//renderer->painter->drawRect(prect_);

  if (renderer->schem->isShowPortText()) {
    renderer->painter->setPen(renderer->textColor);

    for (auto &port : inputs())
      CQSchem::drawTextAtPoint(renderer, port->pixelPos(), port->name());

    for (auto &port : outputs())
      CQSchem::drawTextAtPoint(renderer, port->pixelPos(), port->name());
  }
}

void
CQGate::
drawRect(CQSchemRenderer *renderer) const
{
  renderer->painter->drawRect(prect_);
}

void
CQGate::
drawAnd(CQSchemRenderer *renderer) const
{
  drawAnd(renderer, px1(), py1(), px2(), py2());
}

void
CQGate::
drawAnd(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const
{
  double xm = (x1 + x2)/2.0;
  double ym = (y1 + y2)/2.0;

  QPainterPath path;

  if      (orientation() == Orientation::R0) {
    path.moveTo(x1, y2);
    path.lineTo(x1, y1);
    path.lineTo(xm, y1);
    path.quadTo(x2, y1, x2, ym);
    path.quadTo(x2, y2, xm, y2);
  }
  else if (orientation() == Orientation::R90) {
    path.moveTo(x1, y1);
    path.lineTo(x2, y1);
    path.lineTo(x2, ym);
    path.quadTo(x2, y2, xm, y2);
    path.quadTo(x1, y2, x1, ym);
  }
  else if (orientation() == Orientation::R180) {
    path.moveTo(x2, y1);
    path.lineTo(x2, y2);
    path.lineTo(xm, y2);
    path.quadTo(x1, y2, x1, ym);
    path.quadTo(x1, y1, xm, y1);
  }
  else if (orientation() == Orientation::R270) {
    path.moveTo(x2, y2);
    path.lineTo(x1, y2);
    path.lineTo(x1, ym);
    path.quadTo(x1, y1, xm, y1);
    path.quadTo(x2, y1, x2, ym);
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);
}

void
CQGate::
drawOr(CQSchemRenderer *renderer) const
{
  double x3, y3;

  drawOr(renderer, x3, y3);
}

void
CQGate::
drawOr(CQSchemRenderer *renderer, double &x3, double &y3) const
{
  drawOr(renderer, px1(), py1(), px2(), py2(), x3, y3);
}

void
CQGate::
drawOr(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2,
       double &x3, double &y3) const
{
  double xm = (x1 + x2)/2.0;
  double ym = (y1 + y2)/2.0;

  QPainterPath path;

  if      (orientation() == Orientation::R0) {
    x3 = x1 + (x2 - x1)/4.0;
    y3 = y1;

    double x4 = x2 - (x2 - x1)/4.0;

    path.moveTo(x1, y1);
    path.quadTo(x4, y1, x2, ym);
    path.quadTo(x4, y2, x1, y2);
    path.quadTo(x3, ym, x1, y1);
  }
  else if (orientation() == Orientation::R90) {
    y3 = y1 + (y2 - y1)/4.0;
    x3 = x1;

    double y4 = y2 - (y2 - y1)/4.0;

    path.moveTo(x2, y1);
    path.quadTo(x2, y4, xm, y2);
    path.quadTo(x1, y4, x1, y1);
    path.quadTo(xm, y3, x2, y1);
  }
  else if (orientation() == Orientation::R180) {
    x3 = x2 - (x2 - x1)/4.0;
    y3 = y2;

    double x4 = x1 + (x2 - x1)/4.0;

    path.moveTo(x2, y2);
    path.quadTo(x4, y2, x1, ym);
    path.quadTo(x4, y1, x2, y1);
    path.quadTo(x3, ym, x2, y2);
  }
  else if (orientation() == Orientation::R270) {
    y3 = y2 - (y2 - y1)/4.0;
    x3 = x2;

    double y4 = y1 + (y2 - y1)/4.0;

    path.moveTo(x1, y2);
    path.quadTo(x1, y4, xm, y1);
    path.quadTo(x2, y4, x2, y2);
    path.quadTo(xm, y3, x1, y2);
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);
}

void
CQGate::
drawXor(CQSchemRenderer *renderer) const
{
  drawXor(renderer, px1(), py1(), px2(), py2());
}

void
CQGate::
drawXor(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const
{
  double xm = (x1 + x2)/2.0;
  double ym = (y1 + y2)/2.0;

  double x3, y3;

  drawOr(renderer, x1, y1, x2, y2, x3, y3);

  QPainterPath path;

  if      (orientation() == Orientation::R0) {
    path.moveTo(x1 - 8, y1);
    path.quadTo(x3 - 8, ym, x1 - 8, y2);
  }
  else if (orientation() == Orientation::R90) {
    path.moveTo(x1, y1 - 8);
    path.quadTo(xm, y3 - 8, x2, y1 - 8);
  }
  else if (orientation() == Orientation::R180) {
    path.moveTo(x2 + 8, y1);
    path.quadTo(x3 + 8, ym, x2 + 8, y2);
  }
  else if (orientation() == Orientation::R270) {
    path.moveTo(x1, y2 + 8);
    path.quadTo(xm, y3 + 8, x2, y2 + 8);
  }

  renderer->painter->strokePath(path, renderer->painter->pen());
}

void
CQGate::
drawNot(CQSchemRenderer *renderer) const
{
  QPainterPath path;

  if      (orientation() == Orientation::R0) {
    path.moveTo(px1(), py2());
    path.lineTo(px1(), py1());
    path.lineTo(px2(), pym());
  }
  else if (orientation() == Orientation::R90) {
    path.moveTo(px1(), py1());
    path.lineTo(px2(), py1());
    path.lineTo(pxm(), py2());
  }
  else if (orientation() == Orientation::R180) {
    path.moveTo(px2(), py1());
    path.lineTo(px2(), py2());
    path.lineTo(px1(), pym());
  }
  else if (orientation() == Orientation::R270) {
    path.moveTo(px2(), py2());
    path.lineTo(px1(), py2());
    path.lineTo(pxm(), py1());
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);
}

void
CQGate::
drawNotIndicator(CQSchemRenderer *renderer) const
{
  double ew = renderer->windowWidthToPixelWidth  (0.05);
  double eh = renderer->windowHeightToPixelHeight(0.05);

  QPointF p;

  if      (orientation() == Orientation::R0)
    p = QPointF(px2(), pym());
  else if (orientation() == Orientation::R90)
    p = QPointF(pxm(), py2());
  else if (orientation() == Orientation::R180)
    p = QPointF(px1(), pym());
  else if (orientation() == Orientation::R270)
    p = QPointF(pxm(), py1());

  renderer->painter->drawEllipse(QRectF(p.x() - ew/2, p.y() - eh/2, ew, eh));
}

void
CQGate::
drawAdder(CQSchemRenderer *renderer) const
{
  // draw gate
  double xm1 = (px1() + pxm())/2.0;
  double xm2 = (px2() + pxm())/2.0;
  double ym1 = (py1() + pym())/2.0;
  double ym2 = (py2() + pym())/2.0;

  QPainterPath path;

  if      (orientation() == Orientation::R0) {
    path.moveTo(px1(), py1());
    path.lineTo(px1(), ym1  );
    path.lineTo(xm1  , pym());
    path.lineTo(px1(), ym2  );
    path.lineTo(px1(), py2());
    path.lineTo(px2(), ym2  );
    path.lineTo(px2(), ym1  );
  }
  else if (orientation() == Orientation::R90) {
    path.moveTo(px1(), py1());
    path.lineTo(xm1  , py1());
    path.lineTo(pxm(), ym1  );
    path.lineTo(xm2  , py1());
    path.lineTo(px2(), py1());
    path.lineTo(xm2  , py2());
    path.lineTo(xm1  , py2());
  }
  else if (orientation() == Orientation::R180) {
    path.moveTo(px2(), py1());
    path.lineTo(px2(), ym1  );
    path.lineTo(xm2  , pym());
    path.lineTo(px2(), ym2  );
    path.lineTo(px2(), py2());
    path.lineTo(px1(), ym2  );
    path.lineTo(px1(), ym1  );
  }
  else if (orientation() == Orientation::R270) {
    path.moveTo(px1(), py2());
    path.lineTo(xm1  , py2());
    path.lineTo(pxm(), ym2  );
    path.lineTo(xm2  , py2());
    path.lineTo(px2(), py2());
    path.lineTo(xm2  , py1());
    path.lineTo(xm1  , py1());
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);
}

void
CQGate::
placePorts(int ni, int no) const
{
  placePorts(px1(), py1(), px2(), py2(),
             px1(), py1(), px2(), py2(), ni, no);
}

void
CQGate::
placePorts(double pix1, double piy1, double pix2, double piy2,
           double pox1, double poy1, double pox2, double poy2, int ni, int no) const
{
  if (ni < 0) ni = inputs_ .size();
  if (no < 0) no = outputs_.size();

  assert(ni <= int(inputs_ .size()));
  assert(no <= int(outputs_.size()));

  if (orientation() == Orientation::R0 || orientation() == Orientation::R180) {
    bool flip = (orientation() == Orientation::R180);

    if (isFlipped())
      flip = ! flip;

    std::vector<double> pyi = calcYGaps(piy1, piy2, std::max(ni, 1), flip);
    std::vector<double> pyo = calcYGaps(poy1, poy2, std::max(no, 1), flip);

    double xi = (orientation() == Orientation::R0 ? pix1 : pix2);
    double xo = (orientation() == Orientation::R0 ? pox2 : pox1);

    CQPort::Side iside =
      (orientation() == Orientation::R0 ? CQPort::Side::LEFT  : CQPort::Side::RIGHT);
    CQPort::Side oside =
      (orientation() == Orientation::R0 ? CQPort::Side::RIGHT : CQPort::Side::LEFT );

    for (int i = 0; i < ni; ++i) {
      inputs_[i]->setPixelPos(QPointF(xi, pyi[i]));
      inputs_[i]->setSide(iside);
    }

    for (int i = 0; i < no; ++i) {
      outputs_[i]->setPixelPos(QPointF(xo, pyo[i]));
      outputs_[i]->setSide(oside);
    }
  }
  else {
    bool flip = (orientation() == Orientation::R90);

    if (isFlipped())
      flip = ! flip;

    std::vector<double> pxi = calcXGaps(pix1, pix2, std::max(ni, 1), flip);
    std::vector<double> pxo = calcXGaps(pox1, pox2, std::max(no, 1), flip);

    double yi = (orientation() == Orientation::R90 ? piy1 : piy2);
    double yo = (orientation() == Orientation::R90 ? poy2 : poy1);

    CQPort::Side iside =
      (orientation() == Orientation::R90 ? CQPort::Side::TOP    : CQPort::Side::BOTTOM);
    CQPort::Side oside =
      (orientation() == Orientation::R90 ? CQPort::Side::BOTTOM : CQPort::Side::TOP   );

    for (int i = 0; i < ni; ++i) {
      inputs_[i]->setPixelPos(QPointF(pxi[i], yi));
      inputs_[i]->setSide(iside);
    }

    for (int i = 0; i < no; ++i) {
      outputs_[i]->setPixelPos(QPointF(pxo[i], yo));
      outputs_[i]->setSide(oside);
    }
  }
}

void
CQGate::
placePortOnSide(CQPort *port, const CQPort::Side &side) const
{
  CQPort::Side side1 = CQPort::Side::NONE;

  if      (orientation() == Orientation::R0)
    side1 = side;
  else if (orientation() == Orientation::R90)
    side1 = CQPort::nextSide(side);
  else if (orientation() == Orientation::R180)
    side1 = CQPort::nextSide(CQPort::nextSide(side));
  else if (orientation() == Orientation::R270)
    side1 = CQPort::prevSide(side);

  port->setSide(side1);

  if      (side1 == CQPort::Side::LEFT)
    port->setPixelPos(QPointF(px1(), pym()));
  else if (side1 == CQPort::Side::TOP)
    port->setPixelPos(QPointF(pxm(), py1()));
  else if (side1 == CQPort::Side::RIGHT)
    port->setPixelPos(QPointF(px2(), pym()));
  else if (side1 == CQPort::Side::BOTTOM)
    port->setPixelPos(QPointF(pxm(), py2()));
}

void
CQGate::
placePortsOnSide(CQPort **ports, int n, const CQPort::Side &side) const
{
  CQPort::Side side1 = CQPort::Side::NONE;

  if      (orientation() == Orientation::R0)
    side1 = side;
  else if (orientation() == Orientation::R90)
    side1 = CQPort::nextSide(side);
  else if (orientation() == Orientation::R180)
    side1 = CQPort::nextSide(CQPort::nextSide(side));
  else if (orientation() == Orientation::R270)
    side1 = CQPort::prevSide(side);

  for (int i = 0; i < n; ++i)
    ports[i]->setSide(side1);

  if (side1 == CQPort::Side::LEFT || side1 == CQPort::Side::RIGHT) {
    bool flip = (side1 == CQPort::Side::RIGHT);

    if (isFlipped())
      flip = ! flip;

    std::vector<double> y = calcYGaps(n, flip);

    for (int i = 0; i < n; ++i) {
      if (side1 == CQPort::Side::LEFT)
        ports[i]->setPixelPos(QPointF(px1(), y[i]));
      else
        ports[i]->setPixelPos(QPointF(px2(), y[i]));
    }
  }
  else {
    bool flip = (side1 == CQPort::Side::TOP);

    std::vector<double> x = calcXGaps(n, flip);

    for (int i = 0; i < n; ++i) {
      if (side1 == CQPort::Side::TOP)
        ports[i]->setPixelPos(QPointF(x[i], py1()));
      else
        ports[i]->setPixelPos(QPointF(x[i], py2()));
    }
  }
}

void
CQGate::
connect(const QString &name, CQConnection *connection)
{
  CQPort *port = getPortByName(name);
  assert(port);

  if (port->direction() == CQPort::Direction::IN)
    connection->addOutPort(port);
  else
    connection->addInPort(port);

  port->setConnection(connection);
}

void
CQGate::
addInputPort(const QString &name)
{
  CQPort *port = new CQPort(name, CQPort::Direction::IN);

  port->setGate(this);

  inputs_.push_back(port);
}

void
CQGate::
addOutputPort(const QString &name)
{
  CQPort *port = new CQPort(name, CQPort::Direction::OUT);

  port->setGate(this);

  outputs_.push_back(port);
}

CQPort *
CQGate::
getPortByName(const QString &name) const
{
  for (auto &p : inputs()) {
    if (p->name() == name)
      return p;
  }

  for (auto &p : outputs()) {
    if (p->name() == name)
      return p;
  }

  return nullptr;
}

void
CQGate::
initRect(CQSchemRenderer *renderer) const
{
  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  double xm = xmargin();
  double ym = ymargin();

  double x1 = mapX(xm);
  double x2 = mapX(width() - xm);

  double y1 = mapY(ym);
  double y2 = mapY(height() - ym);

  prect_ = QRectF(x1, y2, x2 - x1, y1 - y2);
}

QColor
CQGate::
penColor(CQSchemRenderer *renderer) const
{
  if (renderer->schem->insideGate() == this)
    return renderer->insideColor;

  return (isSelected() ? renderer->selectColor : renderer->gateStrokeColor);
}

//---

CQNandGate::
CQNandGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
CQNandGate::
exec()
{
  bool b = ! (inputs_[0]->getValue() & inputs_[1]->getValue());

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQNandGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  drawNotIndicator(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQNotGate::
CQNotGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  addInputPort ("a");
  addOutputPort("c");
}

bool
CQNotGate::
exec()
{
  bool b = ! inputs_[0]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQNotGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawNot(renderer);

  drawNotIndicator(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAndGate::
CQAndGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
CQAndGate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAndGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAnd3Gate::
CQAnd3Gate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "c");

  addOutputPort("d");
}

bool
CQAnd3Gate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue() & inputs_[2]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAnd3Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAnd4Gate::
CQAnd4Gate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "c" << "d");

  addOutputPort("e");
}

bool
CQAnd4Gate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue() &
           inputs_[2]->getValue() & inputs_[3]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAnd4Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAnd8Gate::
CQAnd8Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQAnd8Gate::iname(i));

  addOutputPort("o");
}

bool
CQAnd8Gate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue() &
           inputs_[2]->getValue() & inputs_[3]->getValue() &
           inputs_[4]->getValue() & inputs_[5]->getValue() &
           inputs_[6]->getValue() & inputs_[7]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAnd8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQOrGate::
CQOrGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
CQOrGate::
exec()
{
  bool b = inputs_[0]->getValue() | inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQOrGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawOr(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQOr8Gate::
CQOr8Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQOr8Gate::iname(i));

  addOutputPort("o");
}

bool
CQOr8Gate::
exec()
{
  bool b = inputs_[0]->getValue() | inputs_[1]->getValue() |
           inputs_[2]->getValue() | inputs_[3]->getValue() |
           inputs_[4]->getValue() | inputs_[5]->getValue() |
           inputs_[6]->getValue() | inputs_[7]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQOr8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawOr(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQXorGate::
CQXorGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
CQXorGate::
exec()
{
  bool b = inputs_[0]->getValue() ^ inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQXorGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawXor(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQMemoryGate::
CQMemoryGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "i" << "s");

  addOutputPort("o");
}

bool
CQMemoryGate::
exec()
{
  bool s = inputs_[1]->getValue();
  if (! s) return false;

  bool i = inputs_[0]->getValue();

  if (i == state_)
    return false;

  state_ = i;

  outputs_[0]->setValue(state_);

  return true;
}

void
CQMemoryGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  if (sside() == CQPort::Side::LEFT)
    placePorts();
  else {
    placePorts(1, 1);

    placePortOnSide(inputs_[1], sside());
  }

  CQGate::draw(renderer);
}

//---

CQMemory8Gate::
CQMemory8Gate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPort("s");
}

bool
CQMemory8Gate::
exec()
{
  bool s = inputs_[8]->getValue();
  if (! s) return false;

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    bool iv = inputs_[i]->getValue();

    if (iv != state_[i]) {
      state_[i] = iv;

      changed = true;
    }

    outputs_[i]->setValue(state_[i]);
  }

  return changed;
}

void
CQMemory8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts(8, 8);

  // place port s on bottom
  placePortOnSide(inputs_[8], CQPort::Side::BOTTOM);

  CQGate::draw(renderer);
}

//---

CQEnablerGate::
CQEnablerGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = CQEnablerGate::iname(i);
    QString oname = CQEnablerGate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPort("e");
}

bool
CQEnablerGate::
exec()
{
  bool e = inputs_[8]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    bool iv = (e ? inputs_[i]->getValue() : false);

    if (iv != outputs_[i]->getValue()) {
      outputs_[i]->setValue(iv);

      changed = true;
    }
  }

  return changed;
}

void
CQEnablerGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts(8, 8);

  // place port e on bottom
  placePortOnSide(inputs_[8], CQPort::Side::BOTTOM);

  CQGate::draw(renderer);
}

//---

CQRegisterGate::
CQRegisterGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPorts(QStringList() << "s" << "e");
}

bool
CQRegisterGate::
exec()
{
  bool s = inputs_[8]->getValue();
  bool e = inputs_[9]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (s)
      state_[i] = inputs_[i]->getValue();

    bool iv = (e ? state_[i] : false);

    if (iv != outputs_[i]->getValue()) {
      outputs_[i]->setValue(iv);

      changed = true;
    }
  }

  return changed;
}

void
CQRegisterGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts(8, 8);

  // place ports s and e on bottom
  placePortsOnSide(const_cast<CQPort **>(&inputs_[8]), 2, CQPort::Side::BOTTOM);

  CQGate::draw(renderer);
}

//---

CQDecoder4Gate::
CQDecoder4Gate(const QString &name) :
 CQGate(name)
{
  w_ = 2.00/4.0;
  h_ = 1.00;

  addInputPorts(QStringList() << "a" << "b");

  for (int i = 0; i < 4; ++i) {
    QString oname = CQDecoder4Gate::oname(i);

    addOutputPort(oname);
  }
}

bool
CQDecoder4Gate::
exec()
{
  bool a = inputs_[0]->getValue();
  bool b = inputs_[1]->getValue();

  int ab = a | (b << 1);

  bool changed = false;

  for (int i = 0; i < 4; ++i) {
    bool v = (ab == i);

    if (v != outputs_[i]->getValue()) {
      outputs_[i]->setValue(v);

      changed = true;
    }
  }

  return changed;
}

void
CQDecoder4Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQDecoder8Gate::
CQDecoder8Gate(const QString &name) :
 CQGate(name)
{
  w_ = 3.00/8.0;
  h_ = 1.00;

  addInputPorts(QStringList() << "a" << "b" << "c");

  for (int i = 0; i < 8; ++i) {
    QString oname = CQDecoder8Gate::oname(i);

    addOutputPort(oname);
  }
}

bool
CQDecoder8Gate::
exec()
{
  bool a = inputs_[0]->getValue();
  bool b = inputs_[1]->getValue();
  bool c = inputs_[2]->getValue();

  int abc = a | (b << 1) | (c << 2);

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    bool v = (abc == i);

    if (v != outputs_[i]->getValue()) {
      outputs_[i]->setValue(v);

      changed = true;
    }
  }

  return changed;
}

void
CQDecoder8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQDecoder16Gate::
CQDecoder16Gate(const QString &name) :
 CQGate(name)
{
  w_ = 4.00/16.0;
  h_ = 1.0;

  for (int i = 0; i < 4; ++i)
    addInputPort(CQDecoder16Gate::iname(i));

  for (int i = 0; i < 16; ++i)
    addOutputPort(CQDecoder16Gate::oname(i));
}

bool
CQDecoder16Gate::
exec()
{
  bool a = inputs_[0]->getValue();
  bool b = inputs_[1]->getValue();
  bool c = inputs_[2]->getValue();
  bool d = inputs_[3]->getValue();

  int abcd = a | (b << 1) | (c << 2) | (d << 3);

  bool changed = false;

  for (int i = 0; i < 16; ++i) {
    bool v = (abcd == i);

    if (v != outputs_[i]->getValue()) {
      outputs_[i]->setValue(v);

      changed = true;
    }
  }

  return changed;
}

void
CQDecoder16Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQDecoder256Gate::
CQDecoder256Gate(const QString &name) :
 CQGate(name)
{
  w_ = 16.00/256.0;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i)
    addInputPort(CQDecoder256Gate::iname(i));

  for (int i = 0; i < 256; ++i)
    addOutputPort(CQDecoder256Gate::oname(i));
}

bool
CQDecoder256Gate::
exec()
{
  bool a = inputs_[0]->getValue();
  bool b = inputs_[1]->getValue();
  bool c = inputs_[2]->getValue();
  bool d = inputs_[3]->getValue();
  bool e = inputs_[4]->getValue();
  bool f = inputs_[5]->getValue();
  bool g = inputs_[6]->getValue();
  bool h = inputs_[7]->getValue();

  int abcdefgh = a | (b << 1) | (c << 2) | (d << 3) | (e << 4) | (f << 5) | (g << 6) | (h << 7);

  bool changed = false;

  for (int i = 0; i < 256; ++i) {
    bool v = (abcdefgh == i);

    if (v != outputs_[i]->getValue()) {
      outputs_[i]->setValue(v);

      changed = true;
    }
  }

  return changed;
}

void
CQDecoder256Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAdderGate::
CQAdderGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "carry_in");

  addOutputPorts(QStringList() << "sum" << "carry_out");
}

bool
CQAdderGate::
exec()
{
  bool a  = inputs_[0]->getValue();
  bool b  = inputs_[1]->getValue();
  bool ci = inputs_[2]->getValue();

  int ab = int(a) + int(b) + int(ci);

  bool co  = (ab > 1);
  bool sum = (ab & 1);

  bool changed = false;

  if (sum != outputs_[0]->getValue() || co != outputs_[1]->getValue()) {
    outputs_[0]->setValue(sum);
    outputs_[1]->setValue(co);

    changed = true;
  }

  return changed;
}

void
CQAdderGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAdder(renderer);

  //---

  // place ports and draw connections
  placePorts(2, 1);

  // place ports carry_in on top
  placePortOnSide(inputs_[2], CQPort::Side::BOTTOM);

  // place ports carry_in on top
  placePortOnSide(outputs_[1], CQPort::Side::TOP);

  CQGate::draw(renderer);
}

//---

CQAdder8Gate::
CQAdder8Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQAdder8Gate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(CQAdder8Gate::bname(i));

  addInputPort("carry_in");

  for (int i = 0; i < 8; ++i)
    addOutputPort(CQAdder8Gate::cname(i));

  addOutputPort("carry_out");
}

bool
CQAdder8Gate::
exec()
{
  bool ci = inputs_[16]->getValue();
  bool co = 0;

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    bool a = inputs_[i + 0]->getValue();
    bool b = inputs_[i + 8]->getValue();

    int ab = int(a) + int(b) + int(ci);

    bool sum = (ab & 1);

    co = (ab > 1);

    if (sum != outputs_[i]->getValue()) {
      outputs_[i]->setValue(sum);

      changed = true;
    }

    ci = co;
  }

  if (co != outputs_[8]->getValue()) {
    outputs_[8]->setValue(co);

    changed = true;
  }

  return changed;
}

void
CQAdder8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAdder(renderer);

  //---

  // place ports and draw connections
  placePorts(16, 8);

  // place ports carry_in on bottom
  placePortOnSide(inputs_[16], CQPort::Side::BOTTOM);

  // place ports carry_in on top
  placePortOnSide(outputs_[8], CQPort::Side::TOP);

  CQGate::draw(renderer);
}

//---

CQComparatorGate::
CQComparatorGate(const QString &name) :
 CQGate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPorts(QStringList() << "c" << "a_larger" << "equal");
}

bool
CQComparatorGate::
exec()
{
  bool a = inputs_[0]->getValue();
  bool b = inputs_[1]->getValue();

  bool c        = (a != b);
  bool a_larger = (a >  b);
  bool equal    = (a == b);

  bool changed = false;

  if (c        != outputs_[0]->getValue() ||
      a_larger != outputs_[1]->getValue() ||
      equal    != outputs_[2]->getValue()) {
    outputs_[0]->setValue(c);
    outputs_[1]->setValue(a_larger);
    outputs_[2]->setValue(equal);

    changed = true;
  }

  return changed;
}

void
CQComparatorGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawXor(renderer);

  //---

  // place ports and draw connections
  placePorts(2, 1);

  // place ports carry_in on top
  placePortsOnSide(const_cast<CQPort **>(&outputs_[1]), 2, CQPort::Side::TOP);

  CQGate::draw(renderer);
}

//---

CQComparator8Gate::
CQComparator8Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQAdder8Gate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(CQAdder8Gate::bname(i));

  for (int i = 0; i < 8; ++i)
    addOutputPort(CQAdder8Gate::cname(i));

  addOutputPorts(QStringList() << "a_larger" << "equal");
}

bool
CQComparator8Gate::
exec()
{
  int a = 0;
  int b = 0;

  int c[8];

  for (int i = 0; i < 8; ++i) {
    bool a1 = inputs_[i + 0]->getValue();
    bool b1 = inputs_[i + 8]->getValue();

    a |= (a1 << i);
    b |= (b1 << i);

    c[i] = (a1 != b1);
  }

  bool a_larger = (a >  b);
  bool equal    = (a == b);

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (c[i] != outputs_[i]->getValue()) {
      outputs_[i]->setValue(c);

      changed = true;
    }
  }

  if (a_larger != outputs_[8]->getValue() ||
      equal    != outputs_[9]->getValue()) {
    outputs_[8]->setValue(a_larger);
    outputs_[9]->setValue(equal);

    changed = true;
  }

  return changed;
}

void
CQComparator8Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawXor(renderer);

  //---

  // place ports and draw connections
  placePorts(16, 8);

  // place ports carry_in on top
  placePortsOnSide(const_cast<CQPort **>(&outputs_[8]), 2, CQPort::Side::TOP);

  CQGate::draw(renderer);
}

//---

CQBus0Gate::
CQBus0Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQBus0Gate::iname(i));

  addOutputPort("zero");
}

bool
CQBus0Gate::
exec()
{
  bool o = true;

  for (int i = 0; i < 8; ++i) {
    if (inputs_[i]->getValue()) {
      o = false;
      break;
    }
  }

  bool changed = false;

  if (o != outputs_[0]->getValue()) {
    outputs_[0]->setValue(o);

    changed = true;
  }

  return changed;
}

void
CQBus0Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQBus1Gate::
CQBus1Gate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQBus1Gate::iname(i));

  addInputPort("bus1");

  for (int i = 0; i < 8; ++i)
    addOutputPort(CQBus1Gate::oname(i));
}

bool
CQBus1Gate::
exec()
{
  bool bus1 = inputs_[8]->getValue();

  bool o[8];

  if (bus1) {
    for (int i = 0; i < 8; ++i)
      o[i] = (i == 0);
  }
  else {
    for (int i = 0; i < 8; ++i)
      o[i] = inputs_[i]->getValue();
  }

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (o[i] != outputs_[i]->getValue()) {
      outputs_[i]->setValue(o[i]);

      changed = true;
    }
  }

  return changed;
}

void
CQBus1Gate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAluGate::
CQAluGate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(CQAluGate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(CQAluGate::bname(i));

  addInputPort("carry_in");

  for (int i = 0; i < 3; ++i)
    addInputPort(CQAluGate::opname(i));

  for (int i = 0; i < 8; ++i)
    addOutputPort(CQAluGate::cname(i));

  addOutputPort("carry_out");
  addOutputPort("a_larger");
  addOutputPort("equal");
  addOutputPort("zero");
}

bool
CQAluGate::
exec()
{
  bool a[8], b[8];

  for (int i = 0; i < 8; ++i) {
    a[i] = inputs_[i + 0]->getValue();
    b[i] = inputs_[i + 8]->getValue();
  }

  bool carry_in = inputs_[16]->getValue();

  int op = 0;

  for (int i = 0; i < 3; ++i)
    op |= (inputs_[17 + i]->getValue() << i);

  //---

  bool c[8];

  bool carry_out = false;
  bool a_larger  = false;
  bool equal     = false;

  if      (op == 0) { // Add
    for (int i = 0; i < 8; ++i) {
      int ab = int(a[i]) + int(b[i]) + int(carry_in);

      c[i]      = (ab & 1);
      carry_out = (ab > 1);

      carry_in = carry_out;
    }
  }
  else if (op == 1) { // Shift Right
    for (int i = 0; i < 8; ++i) {
      if (i == 0)
        carry_out = a[i];
      else
        c[i - 1] = a[i];
    }

    c[7] = carry_in;
  }
  else if (op == 2) { // Shift Left
    for (int i = 0; i < 8; ++i) {
      if (i == 7)
        carry_out = a[i];
      else
        c[i + 1] = a[i];
    }

    c[0] = carry_in;
  }
  else if (op == 3) { // Not
    for (int i = 0; i < 8; ++i)
      c[i] = ! a[i];
  }
  else if (op == 4) { // And
    for (int i = 0; i < 8; ++i)
      c[i] = a[i] & b[i];
  }
  else if (op == 5) { // Or
    for (int i = 0; i < 8; ++i)
      c[i] = a[i] | b[i];
  }
  else if (op == 6) { // Exclusive Or
    for (int i = 0; i < 8; ++i)
      c[i] = a[i] ^ b[i];
  }
  else if (op == 7) { // Compare
    int a1 = 0;
    int b1 = 0;

    for (int i = 0; i < 8; ++i) {
      a1 |= (a[i] << i);
      b1 |= (b[i] << i);

      c[i] = (a[i] != b[i]);
    }

    a_larger = (a1 >  b1);
    equal    = (a1 == b1);
  }

  bool zero = true;

  for (int i = 0; i < 8; ++i) {
    if (c[i] != 0) {
      zero = false;
      break;
    }
  }

  bool changed = false;

  auto setOutput = [&](int i, bool b) {
    if (b != outputs_[i]->getValue()) {
      outputs_[i]->setValue(b);

      changed = true;
    }
  };

  for (int i = 0; i < 8; ++i)
    setOutput(i, c[i]);

  setOutput( 8, carry_out);
  setOutput( 9, a_larger );
  setOutput(10, equal    );
  setOutput(11, zero     );

  return changed;
}

void
CQAluGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawRect(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQLShiftGate::
CQLShiftGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  addInputPort("shift_in");

  for (int i = 0; i < 8; ++i) {
    addInputPort (iname(i));
    addOutputPort(oname(i));
  }

  addOutputPort("shift_out");

  addInputPorts(QStringList() << "s" << "e");
}

bool
CQLShiftGate::
exec()
{
  bool s = inputs_[ 9]->getValue();
  bool e = inputs_[10]->getValue();

  bool ov[8];

  ov[0] = inputs_[0]->getValue(); // shift_in

  bool so = inputs_[8]->getValue();

  for (int i = 0; i < 7; ++i)
    ov[i + 1] = inputs_[i + 1]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (s)
      state_[i] = ov[i];

    bool iv = (e ? state_[i] : false);

    if (iv != outputs_[i]->getValue()) {
      outputs_[i]->setValue(iv);

      changed = true;
    }
  }

  if (so != outputs_[8]->getValue()) {
    if (s)
      state_[8] = so;

    bool iv = (e ? state_[8] : false);

    if (iv != outputs_[8]->getValue()) {
      outputs_[8]->setValue(so);

      changed = true;
    }
  }

  return changed;
}

void
CQLShiftGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  QPainterPath path;

  double y3 = py1() + (py2() - py1())/5.0;
  double y4 = py2() - (py2() - py1())/5.0;

  double x3 = px1() + (px2() - px1())/5.0;
  double x4 = px2() - (px2() - px1())/5.0;

  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180) {
    path.moveTo(px1(), py2());
    path.lineTo(px2(), y4   );
    path.lineTo(px2(), py1());
    path.lineTo(px1(), y3   );
  }
  else {
    path.moveTo(px1(), py1());
    path.lineTo(x4   , py1());
    path.lineTo(px2(), py2());
    path.lineTo(x3   , py2());
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);

  //---

  // place ports and draw connections
  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180)
    placePorts(px1(), y3, px1(), py2(), px2(), py1(), px2(), y4, 9, 9);
  else
    placePorts(px1(), py1(), x4, py1(), x3, py2(), px2(), py2(), 9, 9);

  // place ports s and e on bottom
  placePortsOnSide(const_cast<CQPort **>(&inputs_[9]), 2, CQPort::Side::BOTTOM);

  CQGate::draw(renderer);
}

//---

CQRShiftGate::
CQRShiftGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  addOutputPort("shift_out");

  for (int i = 0; i < 8; ++i) {
    addInputPort (iname(i));
    addOutputPort(oname(i));
  }

  addInputPort("shift_in");

  addInputPorts(QStringList() << "s" << "e");
}

bool
CQRShiftGate::
exec()
{
  bool s = inputs_[ 9]->getValue();
  bool e = inputs_[10]->getValue();

  bool ov[8];

  bool so = inputs_[0]->getValue(); // bit 0

  for (int i = 0; i < 7; ++i)
    ov[i] = inputs_[i + 1]->getValue(); // bit 1-7

  ov[7] = inputs_[8]->getValue(); // shift_in

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (s)
      state_[i + 1] = ov[i];

    bool iv = (e ? state_[i + 1] : false);

    if (iv != outputs_[i + 1]->getValue()) {
      outputs_[i + 1]->setValue(iv);

      changed = true;
    }
  }

  if (so != outputs_[0]->getValue()) {
    if (s)
      state_[0] = so;

    bool iv = (e ? state_[0] : false);

    if (iv != outputs_[0]->getValue()) {
      outputs_[0]->setValue(iv);

      changed = true;
    }
  }

  return changed;
}

void
CQRShiftGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  QPainterPath path;

  double y3 = py1() + (py2() - py1())/5.0;
  double y4 = py2() - (py2() - py1())/5.0;

  double x3 = px1() + (px2() - px1())/5.0;
  double x4 = px2() - (px2() - px1())/5.0;

  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180) {
    path.moveTo(px1(), py1());
    path.lineTo(px2(), y3   );
    path.lineTo(px2(), py2());
    path.lineTo(px1(), y4   );
  }
  else {
    path.moveTo(px1(), py2());
    path.lineTo(x4   , py2());
    path.lineTo(px2(), py1());
    path.lineTo(x3   , py1());
  }

  path.closeSubpath();

  renderer->painter->setBrush(renderer->gateFillColor);

  renderer->painter->drawPath(path);

  //---

  // place ports and draw connections
  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180)
    placePorts(px1(), py1(), px1(), y4, px2(), y3, px2(), py2(), 9, 9);
  else
    placePorts(x3, py1(), px2(), py1(), px2(), py2(), x4, py2(), 9, 9);

  // place ports s and e on bottom
  placePortsOnSide(const_cast<CQPort **>(&inputs_[9]), 2, CQPort::Side::BOTTOM);

  CQGate::draw(renderer);
}

//---

CQInverterGate::
CQInverterGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  for (int i = 0; i < 8; ++i) {
    addInputPort (iname(i));
    addOutputPort(oname(i));
  }
}

bool
CQInverterGate::
exec()
{
  bool ov[8];

  for (int i = 0; i < 8; ++i)
    ov[i] = ! inputs_[i]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (ov[i] != outputs_[i]->getValue()) {
      outputs_[i]->setValue(ov[i]);

      changed = true;
    }
  }

  return changed;
}

void
CQInverterGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawNot(renderer);

  drawNotIndicator(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQAnderGate::
CQAnderGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  for (int i = 0; i < 8; ++i) {
    addInputPort (aname(i));
    addInputPort (bname(i));
    addOutputPort(cname(i));
  }
}

bool
CQAnderGate::
exec()
{
  bool ov[8];

  for (int i = 0; i < 8; ++i)
    ov[i] = inputs_[2*i + 0]->getValue() & inputs_[2*i + 1]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (ov[i] != outputs_[i]->getValue()) {
      outputs_[i]->setValue(ov[i]);

      changed = true;
    }
  }

  return changed;
}

void
CQAnderGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawAnd(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQOrerGate::
CQOrerGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  for (int i = 0; i < 8; ++i) {
    addInputPort (aname(i));
    addInputPort (bname(i));
    addOutputPort(cname(i));
  }
}

bool
CQOrerGate::
exec()
{
  bool ov[8];

  for (int i = 0; i < 8; ++i)
    ov[i] = inputs_[2*i + 0]->getValue() | inputs_[2*i + 1]->getValue();

  bool changed = false;

  for (int i = 0; i < 8; ++i) {
    if (ov[i] != outputs_[i]->getValue()) {
      outputs_[i]->setValue(ov[i]);

      changed = true;
    }
  }

  return changed;
}

void
CQOrerGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawOr(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//---

CQXorerGate::
CQXorerGate(const QString &name) :
 CQGate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  for (int i = 0; i < 8; ++i) {
    addInputPort (aname(i));
    addInputPort (bname(i));
    addOutputPort(cname(i));
  }
}

bool
CQXorerGate::
exec()
{
  bool ov[8];

  for (int i = 0; i < 8; ++i)
    ov[i] = inputs_[2*i + 0]->getValue() ^ inputs_[2*i + 1]->getValue();

  bool changed = false;

  auto setOutput = [&](int i, bool b) {
    if (b != outputs_[i]->getValue()) {
      outputs_[i]->setValue(b);

      changed = true;
    }
  };

  for (int i = 0; i < 8; ++i)
    setOutput(i, ov[i]);

  return changed;
}

void
CQXorerGate::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  renderer->painter->setPen(penColor(renderer));

  // calc coords
  initRect(renderer);

  //---

  // draw gate
  drawXor(renderer);

  //---

  // place ports and draw connections
  placePorts();

  CQGate::draw(renderer);
}

//------

void
CQPort::
setValue(bool b, bool propagate)
{
  value_ = b;

  if (propagate && connection_)
    connection_->setValue(b);
}

QPointF
CQPort::
offsetPixelPos() const
{
  static int dl = 32;

  Side side = side_;

  if (side == Side::NONE)
    side = (direction_ == Direction::IN ? Side::LEFT : Side::RIGHT);

  int dl1 = dl;

  if (connection_ && (connection_->isInput() || connection_->isOutput()))
    dl1 *= 2;

  if (connection_ && connection_->bus()) {
    int n = connection_->bus()->connectionIndex(connection_);

    dl1 = 8*(n + 1);
  }

  QPointF p;

  if      (side == Side::LEFT)
    p = QPointF(ppos_.x() - dl1, ppos_.y());
  else if (side == Side::RIGHT)
    p = QPointF(ppos_.x() + dl1, ppos_.y());
  else if (side == Side::TOP)
    p = QPointF(ppos_.x(), ppos_.y() - dl1);
  else if (side == Side::BOTTOM)
    p = QPointF(ppos_.x(), ppos_.y() + dl1);
  else
    assert(false);

  return p;
}

//------

CQConnection::
CQConnection(const QString &name) :
 name_(name)
{
}

void
CQConnection::
setValue(bool b)
{
  value_ = b;

  // propagate to output
  for (auto &oport : outPorts())
    oport->setValue(getValue(), false);
}

bool
CQConnection::
isLR() const
{
  CQPort::Side side { CQPort::Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == CQPort::Side::LEFT || side == CQPort::Side::RIGHT);
}

bool
CQConnection::
isLeft() const
{
  CQPort::Side side { CQPort::Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == CQPort::Side::LEFT);
}

bool
CQConnection::
isTop() const
{
  CQPort::Side side { CQPort::Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == CQPort::Side::TOP);
}

bool
CQConnection::
inside(const QPointF &p) const
{
  return prect_.contains(p);
}

void
CQConnection::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isConnectionVisible())
    return;

  lines_.clear();

  renderer->painter->setPen(penColor(renderer));

  double ym = (renderer->rect.top() + renderer->rect.bottom())/2.0;

  QPointF p1(renderer->rect.left (), ym);
  QPointF p2(renderer->rect.right(), ym);

  int ni = inPorts_ .size();
  int no = outPorts_.size();

  if      ((ni > 1 && no >= 1) || (ni >= 1 && no > 1)) {
    // calc average port position
    double xo = 0.0, yo = 0.0;

    for (const auto &port : inPorts_) {
      xo += port->offsetPixelPos().x();
      yo += port->offsetPixelPos().y();
    }

    for (const auto &port : outPorts_) {
      xo += port->offsetPixelPos().x();
      yo += port->offsetPixelPos().y();
    }

    xo /= ni + no;
    yo /= ni + no;

    //---

    for (const auto &port : inPorts_) {
      QPointF p(port->offsetPixelPos().x(), yo);

      addLine(port->pixelPos(), port->offsetPixelPos());
      addLine(port->offsetPixelPos(), p);
      addLine(p, QPointF(xo, yo));
    }

    for (const auto &port : outPorts_) {
      QPointF p(port->offsetPixelPos().x(), yo);

      addLine(QPointF(xo, yo), p);
      addLine(p, port->offsetPixelPos());
      addLine(port->offsetPixelPos(), port->pixelPos());
    }
  }
  else if (ni > 1 && no == 0) {
    // calc average port connection point
    double xo = 0.0;
    double yo = 0.0;

    for (const auto &port : inPorts_) {
      xo += port->offsetPixelPos().x();
      yo += port->offsetPixelPos().y();
    }

    xo /= ni;
    yo /= ni;

    QPointF po(xo, yo);

    // draw lines
    for (const auto &port : inPorts_) {
      addLine(port->pixelPos(), port->offsetPixelPos());

      connectPoints(port->offsetPixelPos(), po);
    }
  }
  else if (no > 1 && ni == 0) {
    // calc average port connection point
    double xo = 0.0;
    double yo = 0.0;

    for (const auto &port : outPorts_) {
      xo += port->offsetPixelPos().x();
      yo += port->offsetPixelPos().y();
    }

    xo /= no;
    yo /= no;

    QPointF po(xo, yo);

    // draw lines
    for (const auto &port : outPorts_) {
      addLine(port->pixelPos(), port->offsetPixelPos());

      connectPoints(port->offsetPixelPos(), po);
    }
  }
  else if (ni == 1 && no == 1) {
    p1 = inPorts_ [0]->pixelPos();
    p2 = outPorts_[0]->pixelPos();

    QPointF p3 = inPorts_ [0]->offsetPixelPos();
    QPointF p4 = outPorts_[0]->offsetPixelPos();

    addConnectLines(p1, p3, p4, p2);
  }
  else if (ni == 1) {
    p1 = inPorts_[0]->pixelPos();
    p2 = inPorts_[0]->offsetPixelPos();

    addLine(p1, p2);
  }
  else if (no == 1) {
    p1 = outPorts_[0]->offsetPixelPos();
    p2 = outPorts_[0]->pixelPos();

    addLine(p1, p2);
  }

  //---

  int    ind  = -1;
  double indX = 0.0;

  if       (isInput()) {
    for (const auto &line : lines_) {
      if (ind == -1 || line.start.x() < indX) {
        ind  = line.ind;
        indX = line.start.x();
      }
    }
  }
  else if (isOutput()) {
    for (const auto &line : lines_) {
      if (ind == -1 || line.start.x() > indX) {
        ind  = line.ind;
        indX = line.start.x();
      }
    }
  }
  else {
    ind = 0;
  }

  for (const auto &line : lines_)
    drawLine(renderer, line.start, line.end, /*showText*/ line.ind == ind);

  //---

//renderer->painter->setPen(Qt::red);
//renderer->painter->drawRect(prect_);
}

void
CQConnection::
addConnectLines(const QPointF &p1, const QPointF &p2,
                const QPointF &p3, const QPointF &p4) const
{
  double dxs = std::abs(p2.x() - p1.x());
  double dys = std::abs(p2.y() - p1.y());

  double dxe = std::abs(p4.x() - p3.x());
  double dye = std::abs(p4.y() - p3.y());

  double dx = std::abs(p4.x() - p1.x());
  double dy = std::abs(p4.y() - p1.y());

  double xm = (p2.x() + p3.x())/2.0;
  double ym = (p2.y() + p3.y())/2.0;

  QPointF p21 = p2;
  QPointF p31 = p3;

  if (dxs > dys)
    p21 = QPointF(std::min(p2.x(), xm), p2.y());
  else
    p21 = QPointF(p2.x(), std::min(p2.y(), ym));

  if (dxe > dye)
    p31 = QPointF(std::max(p3.x(), xm), p3.y());
  else
    p31 = QPointF(p3.x(), std::min(p3.y(), ym));

  if (dx > dy) {
    if (dy > 1) {
      QPointF p5(p21.x(), ym);
      QPointF p6(p31.x(), ym);

      addLine(p1 , p21);
      addLine(p21, p5 );
      addLine(p5 , p6 );
      addLine(p6 , p31);
      addLine(p31, p4 );
    }
    else {
      addLine(p1, p4);
    }
  }
  else {
    if (dx > 1) {
      QPointF p5(xm, p21.y());
      QPointF p6(xm, p31.y());

      addLine(p1 , p21);
      addLine(p21, p5 );
      addLine(p5 , p6 );
      addLine(p6 , p31);
      addLine(p31, p4 );
    }
    else {
      addLine(p1, p4);
    }
  }
}

void
CQConnection::
connectPoints(const QPointF &p1, const QPointF &p2) const
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  if (dx > dy) {
    if (dy > 1) {
      double ym = (p1.y() + p2.y())/2.0;

      QPointF p3(p1.x(), ym);
      QPointF p4(p2.x(), ym);

      addLine(p1, p3);
      addLine(p3, p4);
      addLine(p4, p2);
    }
    else
      addLine(p1, p2);
  }
  else {
    if (dx > 1) {
      double xm = (p1.x() + p2.x())/2.0;

      QPointF p3(xm, p1.y());
      QPointF p4(xm, p2.y());

      addLine(p1, p3);
      addLine(p3, p4);
      addLine(p4, p2);
    }
    else
      addLine(p1, p2);
  }
}

void
CQConnection::
addLine(const QPointF &p1, const QPointF &p2) const
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  assert(int(dx) <= 1 || int(dy) <= 1);

  int ind = lines_.size();

  if (p1.x() > p2.x() || (p1.x() == p2.x() && p1.y() > p2.y()))
    lines_.push_back(Line(ind, p2, p1));
  else
    lines_.push_back(Line(ind, p1, p2));
}

void
CQConnection::
drawLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2,
         bool showText) const
{
  renderer->painter->setPen(penColor(renderer));

  CQSchem::drawLine(renderer, p1, p2);

  if (showText && renderer->schem->isShowConnectionText()) {
    renderer->painter->setPen(renderer->textColor);

    if      (isInput() || isOutput()) {
      if (isLR()) {
        if (isLeft())
          CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::START);
        else
          CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::END);
      }
      else
        CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::MIDDLE);
    }
    else
      CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::MIDDLE);
  }

  if      (p1.y() == p2.y()) {
    double h = 8;

    prect_ = QRectF(p1.x(), p1.y() - h/2, p2.x() - p1.x(), h);
  }
  else if (p1.x() == p2.x()) {
    double w = 8;

    prect_ = QRectF(p1.x() - w/2, p1.y(), w, p2.y() - p1.y());
  }
  else {
    prect_ = QRectF(p1.x(), p1.y(), p2.x(), p1.y());
  }
}

QPointF
CQConnection::
imidPoint() const
{
  int ni = inPorts_ .size();

  if (ni == 0)
    return QPointF();

  double x = 0.0;
  double y = 0.0;

  for (const auto &port : inPorts_) {
    x += port->pixelPos().x();
    y += port->pixelPos().y();
  }

  x /= ni;
  y /= ni;

  return QPointF(x, y);
}

QPointF
CQConnection::
omidPoint() const
{
  int no = outPorts_.size();

  if (no == 0)
    return QPointF();

  double x = 0.0;
  double y = 0.0;

  for (const auto &port : outPorts_) {
    x += port->pixelPos().x();
    y += port->pixelPos().y();
  }

  x /= no;
  y /= no;

  return QPointF(x, y);
}

QPointF
CQConnection::
midPoint() const
{
  int ni = inPorts_ .size();
  int no = outPorts_.size();

  if (ni + no == 0)
    return QPointF();

  double x = 0.0;
  double y = 0.0;

  for (const auto &port : inPorts_) {
    x += port->pixelPos().x();
    y += port->pixelPos().y();
  }

  for (const auto &port : outPorts_) {
    x += port->pixelPos().x();
    y += port->pixelPos().y();
  }

  x /= ni + no;
  y /= ni + no;

  return QPointF(x, y);
}

QColor
CQConnection::
penColor(CQSchemRenderer *renderer) const
{
  if (renderer->schem->insideConnection() == this)
    return renderer->insideColor;

  if (getValue())
    return Qt::green;

  return (isSelected() ? renderer->selectColor : renderer->connectionColor);
}

//------

CQBus::
CQBus(const QString &name, int n) :
 name_(name), n_(n)
{
  connections_.resize(n);
}

void
CQBus::
addConnection(CQConnection *connection, int i)
{
  assert(connection && i >= 0 && i < n_);

  connections_[i] = connection;

  connection->setBus(this);
}

int
CQBus::
connectionIndex(CQConnection *connection)
{
  for (int i = 0; i < n_; ++i)
    if (connections_[i] == connection)
      return i;

  assert(false);

  return -1;
}

void
CQBus::
draw(CQSchemRenderer *renderer)
{
  if (! renderer->schem->isConnectionVisible())
    return;

  auto mapWidth  = [&](double w) { return renderer->windowWidthToPixelWidth  (w); };
  auto mapHeight = [&](double h) { return renderer->windowHeightToPixelHeight(h); };

  //---

  // get is input/output
  bool input  = true;
  bool output = true;

  for (int i = 0; i < n_; ++i) {
    if (! connections_[i]->isInput())
      input = false;

    if (! connections_[i]->isOutput())
      output = false;
  }

  //---

  // calc bus connections min, max and mid points
  double xi1 = 0.0, yi1 = 0.0;
  double xi2 = 0.0, yi2 = 0.0;
  double xis = 0.0, yis = 0.0;
  double xie = 0.0, yie = 0.0;
  double xim = 0.0, yim = 0.0;

  double xo1 = 0.0, yo1 = 0.0;
  double xo2 = 0.0, yo2 = 0.0;
  double xos = 0.0, yos = 0.0;
  double xoe = 0.0, yoe = 0.0;
  double xom = 0.0, yom = 0.0;

  int ni = 0, no = 0;

  for (int i = 0; i < n_; ++i) {
    if (! connections_[i]->isOutput()) {
      QPointF p = connections_[i]->omidPoint();

      xi1 = (i == 0 ? p.x() : std::min(xi1, p.x()));
      yi1 = (i == 0 ? p.y() : std::min(yi1, p.y()));
      xi2 = (i == 0 ? p.x() : std::max(xi2, p.x()));
      yi2 = (i == 0 ? p.y() : std::max(yi2, p.y()));

      xis = (i == 0 ? p.x() : std::min(xis, p.x()));
      yis = (i == 0 ? p.y() : std::min(yis, p.y()));

      xie = (i == 0 ? p.x() : std::max(xie, p.x()));
      yie = (i == 0 ? p.y() : std::max(yie, p.y()));

      xim += p.x();
      yim += p.y();

      ++ni;
    }

    if (! connections_[i]->isInput()) {
      QPointF p = connections_[i]->imidPoint();

      xo1 = (i == 0 ? p.x() : std::min(xo1, p.x()));
      yo1 = (i == 0 ? p.y() : std::min(yo1, p.y()));
      xo2 = (i == 0 ? p.x() : std::max(xo2, p.x()));
      yo2 = (i == 0 ? p.y() : std::max(yo2, p.y()));

      xos = (i == 0 ? p.x() : std::min(xos, p.x()));
      yos = (i == 0 ? p.y() : std::min(yos, p.y()));

      xoe = (i == 0 ? p.x() : std::max(xoe, p.x()));
      yoe = (i == 0 ? p.y() : std::max(yoe, p.y()));

      xom += p.x();
      yom += p.y();

      ++no;
    }
  }

  double xic, yic, xoc, yoc;

  if      (position() == CQBus::Position::START) {
    xic = xis + offset()*(xie - xis);
    yic = yis + offset()*(yie - yis);

    xoc = xos + offset()*(xoe - xis);
    yoc = yos + offset()*(yoe - yis);
  }
  else if (position() == CQBus::Position::END) {
    xic = xie + offset()*(xie - xis);
    yic = yie + offset()*(yie - yis);

    xoc = xoe + offset()*(xoe - xis);
    yoc = yoe + offset()*(yoe - yis);
  }
  else {
    if (ni > 0) {
      xim /= ni;
      yim /= ni;
    }

    if (no > 0) {
      xom /= no;
      yom /= no;
    }

    xic = xim;
    yic = yim;

    xoc = xom;
    yoc = yom;
  }

  //---

  if (renderer->schem->isCollapseBus()) {
    if      (ni > 0 && no == 0) {
      CQPort::Side side = connections_[0]->outPorts()[0]->side();

      bool lr = connections_[0]->isLR();

      QPointF p1, p2;

      if (lr) {
        double dw = mapWidth(renderer->placementRect.width()/2.0);

        if (side == CQPort::Side::LEFT) {
          p1 = QPointF(xi1 - dw, yic);
          p2 = QPointF(xi1     , yic);
        }
        else {
          p1 = QPointF(xi2 + dw, yic);
          p2 = QPointF(xi2     , yic);
        }
      }
      else {
        double dh = mapHeight(renderer->placementRect.height()/2.0);

        if (side == CQPort::Side::TOP) {
          p1 = QPointF(xic, yi1 - dh);
          p2 = QPointF(xic, yi1     );
        }
        else {
          p1 = QPointF(xic, yi1 + dh);
          p2 = QPointF(xic, yi1     );
        }
      }

      CQSchem::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        if (lr) {
          if (side == CQPort::Side::LEFT)
            CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::START);
          else
            CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::END);
        }
        else
          CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::MIDDLE);
      }

      return;
    }
    else if (no > 0 && ni == 0) {
      CQPort::Side side = connections_[0]->inPorts()[0]->side();

      bool lr = connections_[0]->isLR();

      QPointF p1, p2;

      if (lr) {
        double dw = mapWidth(renderer->placementRect.width()/2.0);

        if (side == CQPort::Side::LEFT) {
          p1 = QPointF(xo1 - dw, yoc);
          p2 = QPointF(xo1     , yoc);
        }
        else {
          p1 = QPointF(xo2 + dw, yoc);
          p2 = QPointF(xo2     , yoc);
        }
      }
      else {
        double dh = mapHeight(renderer->placementRect.height()/2.0);

        if (side == CQPort::Side::TOP) {
          p1 = QPointF(xoc, yo1 - dh);
          p2 = QPointF(xoc, yo1     );
        }
        else {
          p1 = QPointF(xoc, yo1 + dh);
          p2 = QPointF(xoc, yo1     );
        }
      }

      CQSchem::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        if (lr) {
          if (side == CQPort::Side::LEFT)
            CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::START);
          else
            CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::END);
        }
        else
          CQSchem::drawTextOnLine(renderer, p1, p2, name(), CQSchem::TextLinePos::MIDDLE);
      }

      return;
    }
    else if (ni > 0 && no > 0) {
      CQPort::Side iside = connections_[0]->inPorts ()[0]->side();
      CQPort::Side oside = connections_[0]->outPorts()[0]->side();

      bool ilr = (iside == CQPort::Side::LEFT || iside == CQPort::Side::RIGHT);
      bool olr = (oside == CQPort::Side::LEFT || oside == CQPort::Side::RIGHT);

      QPointF p1, p2;

      if (ilr) {
        if (iside == CQPort::Side::LEFT) p1 = QPointF(xo1, yoc);
        else                             p1 = QPointF(xo2, yoc);
      }
      else {
        if (iside == CQPort::Side::TOP ) p1 = QPointF(xoc, yo1);
        else                             p1 = QPointF(xoc, yo1);
      }

      if (olr) {
        if (iside == CQPort::Side::LEFT) p2 = QPointF(xi1, yic);
        else                             p2 = QPointF(xi2, yic);
      }
      else {
        if (iside == CQPort::Side::TOP ) p2 = QPointF(xic, yi1);
        else                             p2 = QPointF(xic, yi1);
      }

      CQSchem::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        CQSchem::drawTextAtPoint(renderer,
          QPointF((p1.x() + p2.x())/2, (p1.y() + p2.y())/2), name());
      }

      return;
    }
  }

  //---

  if      (input) {
    CQPort::Side side = connections_[0]->outPorts()[0]->side();

    bool lr = connections_[0]->isLR();

    double dx = (lr ? mapWidth (renderer->placementRect.width ()/2.0) :
                      mapWidth (renderer->placementRect.width ()/32.0));
    double dy = (lr ? mapHeight(renderer->placementRect.height()/32.0) :
                      mapHeight(renderer->placementRect.height()/2.0));

    QPointF p1;

    bool flipped = isFlipped();

    if (gate() && gate()->isFlipped())
      flipped = ! flipped;

    if (lr) {
      if (flipped)
        dy = -dy;

      if (side == CQPort::Side::LEFT)
        p1 = QPointF(xi1 - dx, yic - n_*dy/2.0);
      else
        p1 = QPointF(xi2 + dx, yic + n_*dy/2.0);
    }
    else {
      if (flipped)
        dx = -dx;

      if (side == CQPort::Side::TOP)
        p1 = QPointF(xic + n_*dx/2.0, yi1 - dy);
      else
        p1 = QPointF(xic - n_*dx/2.0, yi2 + dy);
    }

    // count number above, below
    int na = 0, nb = 0;

    for (int i = 0; i < n_; ++i) {
      QPointF p2 = connections_[i]->midPoint();

      if (lr) {
        if (p2.y() < yic) ++na;
        else              ++nb;
      }
      else {
        if (p2.x() < xic) ++na;
        else              ++nb;
      }
    }

    // draw lines
    int ia = 0, ib = 0;

    for (int i = 0; i < n_; ++i) {
      renderer->painter->setPen(connections_[i]->penColor(renderer));

      QPointF p2 = connections_[i]->midPoint();

      QPointF pm1, pm2;

      if (lr) {
        double xm = (p1.x() + p2.x())/2;

        if (p2.y() < yic) {
          pm1 = QPointF(xm + ia*dy/2.0, p1.y());

          ++ia;
        }
        else {
          pm1 = QPointF(xm + (nb - ib - 1)*dy/2.0, p1.y());

          ++ib;
        }

        pm2 = QPointF(pm1.x(), p2.y());
      }
      else {
        double ym = (p1.y() + p2.y())/2;

        if (p2.x() < xic) {
          pm1 = QPointF(p1.x(), ym - ia*dx/2.0);

          ++ia;
        }
        else {
          pm1 = QPointF(p1.x(), ym - (nb - ib - 1)*dx/2.0);

          ++ib;
        }

        pm2 = QPointF(p2.x(), pm1.y());
      }

      CQSchem::drawLine(renderer, p1 , pm1);
      CQSchem::drawLine(renderer, pm1, pm2);
      CQSchem::drawLine(renderer, pm2, p2 );

      for (const auto &port : connections_[i]->outPorts())
        CQSchem::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        CQSchem::drawTextOnLine(renderer, pm2, p2, connections_[i]->name(),
                                CQSchem::TextLinePos::MIDDLE);
      }

      connections_[i]->setPRect(QRectF(p1, p2));

      if (lr) {
        if (side == CQPort::Side::LEFT)
          p1 += QPointF(0, dy);
        else
          p1 -= QPointF(0, dy);
      }
      else {
        if (side == CQPort::Side::TOP)
          p1 -= QPointF(dx, 0);
        else
          p1 += QPointF(dx, 0);
      }
    }
  }
  else if (output) {
    CQPort::Side side = connections_[0]->inPorts()[0]->side();

    bool lr = connections_[0]->isLR();

    double dx = (lr ? mapWidth (renderer->placementRect.width ()/2.0) :
                      mapWidth (renderer->placementRect.width ()/32.0));
    double dy = (lr ? mapHeight(renderer->placementRect.height()/32.0) :
                      mapHeight(renderer->placementRect.height()/2.0));

    QPointF p1;

    bool flipped = isFlipped();

    if (gate() && gate()->isFlipped())
      flipped = ! flipped;

    if (lr) {
      if (flipped)
        dy = -dy;

      if (side == CQPort::Side::LEFT)
        p1 = QPointF(xo1 - dx, yoc + n_*dy/2.0);
      else
        p1 = QPointF(xo2 + dx, yoc - n_*dy/2.0);
    }
    else {
      if (flipped)
        dx = -dx;

      if (side == CQPort::Side::TOP)
        p1 = QPointF(xoc - n_*dx/2.0, yo1 - dy);
      else
        p1 = QPointF(xoc + n_*dx/2.0, yo2 + dy);
    }

    // count number above, below
    int na = 0, nb = 0;

    for (int i = 0; i < n_; ++i) {
      QPointF p2 = connections_[i]->midPoint();

      if (lr) {
        if (p2.y() < yoc) ++na;
        else              ++nb;
      }
      else {
        if (p2.x() < xoc) ++na;
        else              ++nb;
      }
    }

    // draw lines
    int ia = 0, ib = 0;

    for (int i = 0; i < n_; ++i) {
      renderer->painter->setPen(connections_[i]->penColor(renderer));

      QPointF p2 = connections_[i]->midPoint();

      QPointF pm1, pm2;

      if (lr) {
        double xm = (p1.x() + p2.x())/2;

        if (p2.y() < yoc) {
          pm1 = QPointF(xm - ia*dy/2.0, p1.y());

          ++ia;
        }
        else {
          pm1 = QPointF(xm - (nb - ib - 1)*dy/2.0, p1.y());

          ++ib;
        }

        pm2 = QPointF(pm1.x(), p2.y());
      }
      else {
        double ym = (p1.y() + p2.y())/2;

        if (p2.x() < xoc) {
          pm1 = QPointF(p1.x(), ym + ia*dx/2.0);

          ++ia;
        }
        else {
          pm1 = QPointF(p1.x(), ym + (nb - ib - 1)*dx/2.0);

          ++ib;
        }

        pm2 = QPointF(p2.x(), pm1.y());
      }

      CQSchem::drawLine(renderer, p1 , pm1);
      CQSchem::drawLine(renderer, pm1, pm2);
      CQSchem::drawLine(renderer, pm2, p2 );

      for (const auto &port : connections_[i]->inPorts())
        CQSchem::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        CQSchem::drawTextOnLine(renderer, pm2, p2, connections_[i]->name(),
                                CQSchem::TextLinePos::MIDDLE);
      }

      connections_[i]->setPRect(QRectF(p1, p2));

      if (lr) {
        if (side == CQPort::Side::LEFT)
          p1 -= QPointF(0, dy);
        else
          p1 += QPointF(0, dy);
      }
      else {
        if (side == CQPort::Side::TOP)
          p1 += QPointF(dx, 0);
        else
          p1 -= QPointF(dx, 0);
      }
    }
  }
  else {
    for (int i = 0; i < n_; ++i)
      connections_[i]->draw(renderer);
  }
}

//------

CQPlacementGroup::
CQPlacementGroup(const Placement &placement, int nr, int nc) :
 placement_(placement), nr_(nr), nc_(nc)
{
}

CQPlacementGroup::
~CQPlacementGroup()
{
  for (auto &placementGroup : placementGroups_)
    delete placementGroup.placementGroup;
}

void
CQPlacementGroup::
setRect(const QRectF &r)
{
  updateRect();

  double dx = r.left() - rect_.left();
  double dy = r.top () - rect_.top ();

  rect_ = r;

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->setRect(placementGroup->rect().translated(dx, dy));
  }

  for (auto &gateData : gates_) {
    CQGate *gate = gateData.gate;

    gate->setRect(gate->rect().translated(dx, dy));
  }
}

void
CQPlacementGroup::
addGate(CQGate *gate, int r, int c, int nr, int nc, Alignment alignment)
{
  if (placement_ == Placement::GRID) {
    assert(r >=0 && r < nr_);
    assert(c >=0 && c < nc_);

    int r1 = r + nr - 1;
    int c1 = c + nc - 1;

    assert(r1 >=0 && r1 < nr_);
    assert(c1 >=0 && c1 < nc_);
  }

  if (gate->placementGroup())
    gate->placementGroup()->removeGate(gate);

  gates_.push_back(GateData(gate, r, c, nr, nc, alignment));

  gate->setPlacementGroup(this);

  rectValid_ = false;
}

void
CQPlacementGroup::
removeGate(CQGate *gate)
{
  int i = 0;
  int n = gates_.size();

  for ( ; i < n; ++i) {
    if (gates_[i].gate == gate)
      break;
  }

  assert(i < n);

  ++i;

  for ( ; i < n; ++i)
    gates_[i - 1] = gates_[i];

  gates_.pop_back();

  rectValid_ = false;
}

void
CQPlacementGroup::
addConnection(CQConnection *connection)
{
  connections_.push_back(connection);
}

void
CQPlacementGroup::
removeConnection(CQConnection *connection)
{
  int i = 0;
  int n = connections_.size();

  for ( ; i < n; ++i) {
    if (connections_[i] == connection)
      break;
  }

  assert(i < n);

  ++i;

  for ( ; i < n; ++i)
    connections_[i - 1] = connections_[i];

  connections_.pop_back();
}

void
CQPlacementGroup::
addBus(CQBus *bus)
{
  buses_.push_back(bus);
}

void
CQPlacementGroup::
removeBus(CQBus *bus)
{
  int i = 0;
  int n = buses_.size();

  for ( ; i < n; ++i) {
    if (buses_[i] == bus)
      break;
  }

  assert(i < n);

  ++i;

  for ( ; i < n; ++i)
    buses_[i - 1] = buses_[i];

  buses_.pop_back();
}

CQPlacementGroup *
CQPlacementGroup::
addPlacementGroup(const Placement &placement, int nr, int nc, int r1, int c1, int nr1, int nc1,
                  Alignment alignment)
{
  CQPlacementGroup *placementGroup = new CQPlacementGroup(placement, nr, nc);

  addPlacementGroup(placementGroup, r1, c1, nr1, nc1, alignment);

  return placementGroup;
}

void
CQPlacementGroup::
addPlacementGroup(CQPlacementGroup *placementGroup, int r, int c, int nr, int nc,
                  Alignment alignment)
{
  placementGroups_.push_back(PlacementGroupData(placementGroup, r, c, nr, nc, alignment));

  placementGroup->parentPlacementGroup_ = this;

  rectValid_ = false;
}

void
CQPlacementGroup::
replacePlacementGroup(CQSchem *schem, CQPlacementGroup *placementGroup)
{
  // find position
  int i = 0;
  int n = placementGroups_.size();

  for ( ; i < n - 1; ++i) {
    if (placementGroups_[i].placementGroup == placementGroup)
      break;
  }

  assert(i < n - 1);

  //---

  // delete old placement
  CQPlacementGroup *oldGroup = placementGroups_[i++].placementGroup;

  for (auto &gateData : oldGroup->gates_)
    schem->removeGate(gateData.gate);

  for (auto &connection : oldGroup->connections_)
    schem->removeConnection(connection);

  for (auto &bus : oldGroup->buses_)
    schem->removeBus(bus);

  delete oldGroup;

  //---

  for ( ; i < n; ++i)
    placementGroups_[i - 1] = placementGroups_[i];

  placementGroups_.pop_back();
}

void
CQPlacementGroup::
updateRect() const
{
  if (rectValid_)
    return;

  CQPlacementGroup *th = const_cast<CQPlacementGroup *>(this);

  th->rect_ = QRectF();

  for (auto &gateData : gates_) {
    CQGate *gate = gateData.gate;

    if (! th->rect_.isEmpty())
      th->rect_ = th->rect_.united(gate->rect());
    else
      th->rect_ = gate->rect();
  }

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (! th->rect_.isEmpty())
      th->rect_ = th->rect_.united(placementGroup->rect());
    else
      th->rect_ = placementGroup->rect();
  }
}

QColor
CQPlacementGroup::
penColor(CQSchemRenderer *renderer) const
{
  if (renderer->schem->insidePlacement() == this)
    return renderer->insideColor;

  return (isSelected() ? renderer->selectColor : QColor(150,150,250));
}

QSizeF
CQPlacementGroup::
calcSize() const
{
  const_cast<CQPlacementGroup *>(this)->place();

  return QSizeF(w_, h_);
}

void
CQPlacementGroup::
place()
{
  w_ = 0.0;
  h_ = 0.0;

  //---

  int nc = placementGroups_.size() + gates_.size();

  if (nc_ < 0)
    nc_ = std::max(int(sqrt(nc)), 1);

  if (nr_ < 0)
    nr_ = std::max((nc + nc_ - 1)/nc_, 1);

  //----

  int r = 0;
  int c = 0;

  std::vector<double> rowHeights, colWidths;

  rowHeights.resize(nr_);
  colWidths .resize(nc_);

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    QSizeF size = placementGroup->calcSize();

    if     (placement() == Placement::HORIZONTAL) {
      w_ += size.width();
      h_  = std::max(h_, size.height());
    }
    else if (placement() == Placement::VERTICAL) {
      w_  = std::max(w_, size.width());
      h_ += size.height();
    }
    else if (placement() == Placement::GRID) {
      int r1 = (placementGroupData.r >= 0 ? placementGroupData.r : r);
      int c1 = (placementGroupData.c >= 0 ? placementGroupData.c : c);

      assert(r1 >= 0 && r1 < nr_);
      assert(c1 >= 0 && c1 < nc_);

      int nr1 = placementGroupData.nr;
      int nc1 = placementGroupData.nc;

      for (int ic = 0; ic < nc1; ++ic)
        colWidths[c1 + ic] = std::max(colWidths[c1 + ic], size.width()/nc1);

      for (int ir = 0; ir < nr1; ++ir)
        rowHeights[r1 + ir] = std::max(rowHeights[r1 + ir], size.height()/nr1);

      if (placementGroupData.c < 0) {
        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;
        }
      }
    }
    else {
      assert(false);
    }
  }

  //--

  for (auto &gateData : gates_) {
    CQGate *gate = gateData.gate;

    QSizeF size = gate->calcSize();

    if     (placement() == Placement::HORIZONTAL) {
      w_ += size.width();
      h_  = std::max(h_, size.height());
    }
    else if (placement() == Placement::VERTICAL) {
      w_  = std::max(w_, size.width());
      h_ += size.height();
    }
    else if (placement() == Placement::GRID) {
      int r1 = (gateData.r >= 0 ? gateData.r : r);
      int c1 = (gateData.c >= 0 ? gateData.c : c);

      assert(r1 >= 0 && r1 < nr_);
      assert(c1 >= 0 && c1 < nc_);

      int nr1 = gateData.nr;
      int nc1 = gateData.nc;

      for (int ic = 0; ic < nc1; ++ic)
        colWidths[c1 + ic] = std::max(colWidths[c1 + ic], size.width()/nc1);

      for (int ir = 0; ir < nr1; ++ir)
        rowHeights[r1 + ir] = std::max(rowHeights[r1 + ir], size.height()/nr1);

      if (gateData.c < 0) {
        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;
        }
      }
    }
    else {
      assert(false);
    }
  }

  //--

  if (placement() == Placement::GRID) {
    w_ = 0.0;
    h_ = 0.0;

    for (auto &colWidth : colWidths)
      w_ += colWidth;

    for (auto &rowHeight : rowHeights)
      h_ += rowHeight;
  }

  //----

  double x = 0.0;
  double y = 0.0;

  r = 0;
  c = 0;

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    QSizeF size = placementGroup->calcSize();

    QRectF rect;

    if     (placement() == Placement::HORIZONTAL) {
      rect = QRectF(x, (h_ - size.height())/2.0, size.width(), size.height());

      x += size.width();
    }
    else if (placement() == Placement::VERTICAL) {
      rect = QRectF((w_ - size.width())/2.0, y, size.width(), size.height());

      y += size.height();
    }
    else if (placement() == Placement::GRID) {
      int r1 = (placementGroupData.r >= 0 ? placementGroupData.r : r);
      int c1 = (placementGroupData.c >= 0 ? placementGroupData.c : c);

      int nr1 = placementGroupData.nr;
      int nc1 = placementGroupData.nc;

      double x1 = x;
      double y1 = y;

      if (placementGroupData.r >= 0) {
        y1 = 0.0;

        for (int r = 0; r < placementGroupData.r; ++r)
          y1 += rowHeights[r];
      }

      if (placementGroupData.c >= 0) {
        x1 = 0.0;

        for (int c = 0; c < placementGroupData.c; ++c)
          x1 += colWidths[c];
      }

      double w1 = 0.0;
      double h1 = 0.0;

      for (int ic = 0; ic < nc1; ++ic)
        w1 += colWidths[c1 + ic];

      for (int ir = 0; ir < nr1; ++ir)
        h1 += rowHeights[r1 + ir];

      double w2 = size.width ();
      double h2 = size.height();

      if (placementGroupData.alignment == CQPlacementGroup::Alignment::HFILL ||
          placementGroupData.alignment == CQPlacementGroup::Alignment::FILL)
        w2 = w1;

      if (placementGroupData.alignment == CQPlacementGroup::Alignment::VFILL ||
          placementGroupData.alignment == CQPlacementGroup::Alignment::FILL)
        h2 = h1;

      rect = QRectF(x1 + (w1 - w2)/2.0, y1 + (h1 - h2)/2.0, w2, h2);

      if (placementGroupData.c < 0) {
        x += colWidths[c];

        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;

          x = 0.0;
          y += rowHeights[r];
        }
      }
    }
    else {
      assert(false);
    }

    placementGroup->setRect(rect);
  }

  //--

  for (auto &gateData : gates_) {
    CQGate *gate = gateData.gate;

    QSizeF size = gate->calcSize();

    QRectF rect;

    if     (placement() == Placement::HORIZONTAL) {
      rect = QRectF(x, (h_ - size.height())/2.0, size.width(), size.height());

      x += size.width();
    }
    else if (placement() == Placement::VERTICAL) {
      rect = QRectF((w_ - size.width())/2.0, y, size.width(), size.height());

      y += size.height();
    }
    else if (placement() == Placement::GRID) {
      int r1 = (gateData.r >= 0 ? gateData.r : r);
      int c1 = (gateData.c >= 0 ? gateData.c : c);

      int nr1 = gateData.nr;
      int nc1 = gateData.nc;

      double x1 = x;
      double y1 = y;

      if (gateData.r >= 0) {
        y1 = 0.0;

        for (int r = 0; r < gateData.r; ++r)
          y1 += rowHeights[r];
      }

      if (gateData.c >= 0) {
        x1 = 0.0;

        for (int c = 0; c < gateData.c; ++c)
          x1 += colWidths[c];
      }

      double w1 = 0.0;
      double h1 = 0.0;

      for (int ic = 0; ic < nc1; ++ic)
        w1 += colWidths[c1 + ic];

      for (int ir = 0; ir < nr1; ++ir)
        h1 += rowHeights[r1 + ir];

      double w2 = size.width ();
      double h2 = size.height();

      if (gateData.alignment == CQPlacementGroup::Alignment::HFILL ||
          gateData.alignment == CQPlacementGroup::Alignment::FILL)
        w2 = w1;

      if (gateData.alignment == CQPlacementGroup::Alignment::VFILL ||
          gateData.alignment == CQPlacementGroup::Alignment::FILL)
        h2 = h1;

      rect = QRectF(x1 + (w1 - w2)/2.0, y1 + (h1 - h2)/2.0, w2, h2);

      if (gateData.c < 0) {
        x += colWidths[c];

        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;

          x = 0.0;
          y += rowHeights[r];
        }
      }
    }
    else {
      assert(false);
    }

    gate->setRect(rect);

    if (gateData.alignment == CQPlacementGroup::Alignment::HFILL ||
        gateData.alignment == CQPlacementGroup::Alignment::FILL)
      gate->setWidth(rect.width());

    if (gateData.alignment == CQPlacementGroup::Alignment::VFILL ||
        gateData.alignment == CQPlacementGroup::Alignment::FILL)
      gate->setHeight(rect.height());
  }

  //---

  updateRect();
  //setRect(QRectF(0, 0, w_, h_));
}

void
CQPlacementGroup::
draw(CQSchemRenderer *renderer) const
{
  if (! renderer->schem->isPlacementGroupVisible())
    return;

  const_cast<CQPlacementGroup *>(this)->updateRect();

  renderer->painter->setPen(penColor(renderer));
  renderer->painter->setBrush(Qt::NoBrush);

  prect_ = renderer->windowToPixel(rect());

  renderer->painter->drawRect(prect_);

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->draw(renderer);
  }
}
