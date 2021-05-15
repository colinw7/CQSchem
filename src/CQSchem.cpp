#include <CQSchem.h>
#include <CQIconButton.h>

#include <QApplication>
#include <QSplitter>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QTimer>

#include <set>
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
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>
#include <svg/play_svg.h>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  //---

  QFont font = app.font();

  double s = font.pointSizeF();

  font.setPointSizeF(s*1.5);

  app.setFont(font);

  //---

  bool test     = false;
  bool waveform = false;

  std::vector<std::string> gates;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg = &argv[i][1];

      if      (arg == "test")
        test = true;
      else if (arg == "waveform")
        waveform = true;
      else
        gates.push_back(arg);
    }
  }

  auto *window = new CQSchem::Window(waveform);

  auto *schem = window->schem();

  for (const auto &gate : gates) {
    if (schem->execGate(gate.c_str()))
      continue;

    std::cerr << "Invalid arg '-" << gate << "'\n";
  }

  schem->place();

  if (test) {
    schem->exec();

    schem->test();
  }
  else {
    window->show();

    app.exec();
  }
}

//------

namespace CQSchem {

Window::
Window(bool waveform)
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

  if (waveform) {
    splitter_ = new QSplitter(Qt::Vertical);
    splitter_->setObjectName("splitter");

    layout->addWidget(splitter_);
  }

  //---

  schem_ = new Schematic(this);

  if (waveform)
    splitter_->addWidget(schem_);
  else
    layout->addWidget(schem_);

  //--

  auto addCheckButton = [&](const QString &name, const QString &iconName, bool checked,
                            const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setCheckable(true);
    button->setChecked(checked);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked(bool)), this, slotName);

    return button;
  };

  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto *connectionTextButton =
    addCheckButton("connectionText", "CONNECTION_TEXT", schem_->isShowConnectionText(),
                   "Connection Text", SLOT(connectionTextSlot(bool)));
  auto *gateTextButton =
    addCheckButton("gateText"      , "GATE_TEXT"      , schem_->isShowGateText(),
                   "Gate Text", SLOT(gateTextSlot(bool)));
  auto *portTextButton =
    addCheckButton("portText"      , "PORT_TEXT"      , schem_->isShowPortText(),
                   "Port Text", SLOT(portTextSlot(bool)));

  auto *moveGateButton =
    addCheckButton("moveGate"      , "MOVE_GATE"      , schem_->isMoveGate(),
                   "Move Gate", SLOT(moveGateSlot(bool)));
  auto *movePlacementButton =
    addCheckButton("movePlacement" , "MOVE_PLACEMENT" , schem_->isMovePlacement(),
                   "Move Placement", SLOT(movePlacementSlot(bool)));
  auto *moveConnectionButton =
    addCheckButton("moveConnection", "MOVE_CONNECTION", schem_->isMoveConnection(),
                   "Move Connection", SLOT(moveConnectionSlot(bool)));

  auto *connectionVisibleButton =
    addCheckButton("connectionVisible", "CONNECTION_VISIBLE",
                   schem_->isConnectionVisible(), "Connection Visible",
                   SLOT(connectionVisibleSlot(bool)));
  auto *gateVisibleButton =
    addCheckButton("gateVisible", "GATE_VISIBLE",
                   schem_->isGateVisible(), "Gate Visible",
                   SLOT(gateVisibleSlot(bool)));
  auto *placementGroupVisibleButton =
    addCheckButton("placementGroupVisible", "PLACEMENT_GROUP_VISIBLE",
                   schem_->isPlacementGroupVisible(), "Placement Group Visible",
                   SLOT(placementGroupVisibleSlot(bool)));

  auto *collapseBusButton =
    addCheckButton("collapseBus", "COLLAPSE_BUS",
                   schem_->isCollapseBus(), "Collapse Bus",
                   SLOT(collapseBusSlot(bool)));

  playButton_  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  pauseButton_ = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  stepButton_  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  pauseButton_->setEnabled(false);

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

  controlLayout->addWidget(playButton_);
  controlLayout->addWidget(pauseButton_);
  controlLayout->addWidget(stepButton_);

  controlLayout->addStretch(1);

  //---

  if (waveform) {
    waveform_ = new Waveform(schem_);

    splitter_->addWidget(waveform_);
  }

  //---

  QFrame *statusFrame = new QFrame;
  statusFrame->setObjectName("statusFrame");

  statusFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  layout->addWidget(statusFrame);

  QHBoxLayout *statusLayout = new QHBoxLayout(statusFrame);

  posLabel_ = new QLabel;

  statusLayout->addStretch(1);
  statusLayout->addWidget(posLabel_);

  //---

  timer_ = new QTimer;

  connect(timer_, SIGNAL(timeout()), this, SLOT(timerSlot()));
}

void
Window::
timerSlot()
{
  schem_->exec();
}

void
Window::
setPos(const QPointF &pos)
{
  posLabel_->setText(QString("%1,%2").arg(pos.x()).arg(pos.y()));
}

void
Window::
connectionTextSlot(bool b)
{
  schem_->setShowConnectionText(b);
}

void
Window::
gateTextSlot(bool b)
{
  schem_->setShowGateText(b);
}

void
Window::
portTextSlot(bool b)
{
  schem_->setShowPortText(b);
}

void
Window::
moveGateSlot(bool b)
{
  schem_->setMoveGate(b);

  if (b) {
    schem_->setMovePlacement (! b);
    schem_->setMoveConnection(! b);
  }
}

void
Window::
movePlacementSlot(bool b)
{
  schem_->setMovePlacement(b);

  if (b) {
    schem_->setMoveGate      (! b);
    schem_->setMoveConnection(! b);
  }
}

void
Window::
moveConnectionSlot(bool b)
{
  schem_->setMoveConnection(b);

  if (b) {
    schem_->setMoveGate     (! b);
    schem_->setMovePlacement(! b);
  }
}

void
Window::
connectionVisibleSlot(bool b)
{
  schem_->setConnectionVisible(b);

  redraw();
}

void
Window::
gateVisibleSlot(bool b)
{
  schem_->setGateVisible(b);

  redraw();
}

void
Window::
placementGroupVisibleSlot(bool b)
{
  schem_->setPlacementGroupVisible(b);

  redraw();
}

void
Window::
collapseBusSlot(bool b)
{
  schem_->setCollapseBus(b);

  redraw();
}

void
Window::
playSlot()
{
  if (! timerActive_) {
    timer_->start(50);

    timerActive_ = true;
  }

  playButton_ ->setEnabled(! timerActive_);
  pauseButton_->setEnabled(timerActive_);
}

void
Window::
pauseSlot()
{
  if (timerActive_) {
    timer_->stop();

    timerActive_ = false;
  }

  playButton_ ->setEnabled(! timerActive_);
  pauseButton_->setEnabled(timerActive_);
}

void
Window::
stepSlot()
{
  schem_->exec();
}

void
Window::
redraw()
{
  schem_->redraw();

  update();
}

QSize
Window::
sizeHint() const
{
  return QSize(800, 800);
}

//------

Schematic::
Schematic(Window *window) :
 window_(window)
{
  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setMouseTracking(true);

  //---

  placementGroup_ = new PlacementGroup;

  debugConnect_ = (getenv("CQSCHEM_DEBUG_CONNECT") != nullptr);
}

Schematic::
~Schematic()
{
  clear();
}

void
Schematic::
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

  placementGroup_ = new PlacementGroup;

  rect_ = QRectF();

  pressGate_        = nullptr;
  pressPlacement_   = nullptr;
  insideGate_       = nullptr;
  insidePlacement_  = nullptr;
  insideConnection_ = nullptr;
}

bool
Schematic::
execGate(const QString &name)
{
  return execGate(placementGroup_, name);
}

bool
Schematic::
execGate(PlacementGroup *parentGroup, const QString &name)
{
  PlacementGroup *oldPlacementGroup = placementGroup_;

  placementGroup_ = parentGroup;

  bool rc = true;

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
  else if (name == "stepper"    ) addStepperGate();
  else if (name == "clk_es"     ) addClkESGate();

  else if (name.startsWith("clk")) {
    QStringList strs = name.mid(3).split(':');

    int delay = 0;
    int cycle = 0;

    if (strs.size() > 0)
      delay = std::max(strs[0].toInt(), 0);

    if (strs.size() > 1)
      cycle = std::max(strs[1].toInt(), 0);

    addClkGate(delay, cycle);
  }

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
  else if (name == "build_clk"        ) buildClk();
  else if (name == "build_clk_es"     ) buildClkES();
  else if (name == "build_stepper"    ) buildStepper();
  else if (name == "build_control1"   ) buildControl1();
  else if (name == "build_control2"   ) buildControl2();
  else if (name == "build_control3"   ) buildControl3();
  else if (name == "build_control4"   ) buildControl4();
  else if (name == "build_control5"   ) buildControl5();

  else if (name == "test_connection") testConnection();

  else rc = false;

  placementGroup_ = oldPlacementGroup;

  return rc;
}

void
Schematic::
addNandGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NandGate *gate = addGateT<NandGate>("nand");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
Schematic::
addNotGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_not");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NotGate *gate = addGateT<NotGate>("not");

  placementGroup->addGate(gate);

  Connection *in  = addPlacementConn("a");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in );
  gate->connect("c", out);
}

void
Schematic::
addAndGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  AndGate *gate = addGateT<AndGate>("and");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
Schematic::
addAnd3Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and3");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  And3Gate *gate = addGateT<And3Gate>("and3");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *in3 = addPlacementConn("c");
  Connection *out = addPlacementConn("d");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", out);
}

void
Schematic::
addAnd4Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  And4Gate *gate = addGateT<And4Gate>("and4");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *in3 = addPlacementConn("c");
  Connection *in4 = addPlacementConn("d");
  Connection *out = addPlacementConn("e");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", in4);
  gate->connect("e", out);
}

void
Schematic::
addAnd8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_and8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  And8Gate *gate = addGateT<And8Gate>("and8");

  placementGroup->addGate(gate);

  Connection *in[8];

  Bus *ibus = addPlacementBus("i", 8);

  for (int i = 0; i < 8; ++i) {
    in[i] = addPlacementConn(And8Gate::iname(i));

    ibus->addConnection(in[i], i);
  }

  Connection *out = addPlacementConn("o");

  for (int i = 0; i < 8; ++i)
    gate->connect(And8Gate::iname(i), in[i]);

  gate->connect("o", out);
}

void
Schematic::
addOrGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_or");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  OrGate *gate = addGateT<OrGate>("or");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
Schematic::
addOr8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_or8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Or8Gate *gate = addGateT<Or8Gate>("or8");

  placementGroup->addGate(gate);

  Connection *in[8];

  Bus *ibus = addPlacementBus("i", 8);

  for (int i = 0; i < 8; ++i) {
    in[i] = addPlacementConn(Or8Gate::iname(i));

    ibus->addConnection(in[i], i);
  }

  Connection *out = addPlacementConn("o");

  for (int i = 0; i < 8; ++i)
    gate->connect(Or8Gate::iname(i), in[i]);

  gate->connect("o", out);
}

void
Schematic::
addXorGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_xor");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  XorGate *gate = addGateT<XorGate>("xor");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
Schematic::
addMemoryGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_memory");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  MemoryGate *gate = addGateT<MemoryGate>("M");

  placementGroup->addGate(gate);

  Connection *in1 = addPlacementConn("i");
  Connection *in2 = addPlacementConn("s");
  Connection *out = addPlacementConn("o");

  gate->connect("i", in1);
  gate->connect("s", in2);
  gate->connect("o", out);
}

void
Schematic::
addMemory8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_memory8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Memory8Gate *gate = addGateT<Memory8Gate>("B");

  placementGroup->addGate(gate);

  Connection *cons = addPlacementConn("s");

  gate->connect("s", cons);

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *coni[8];
  Connection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = Memory8Gate::iname(i);
    QString oname = Memory8Gate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }
}

void
Schematic::
addEnablerGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_enabler");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  EnablerGate *gate = addGateT<EnablerGate>("E");

  placementGroup->addGate(gate);

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *coni[8];
  Connection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = EnablerGate::iname(i);
    QString oname = EnablerGate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }

  Connection *cone = addPlacementConn("e");

  gate->connect("e", cone);
}

void
Schematic::
addRegisterGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  RegisterGate *gate = addGateT<RegisterGate>("R");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *coni[8];
  Connection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    coni[i] = addPlacementConn(iname);
    cono[i] = addPlacementConn(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }
}

void
Schematic::
addDecoder4Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Decoder4Gate *gate = addGateT<Decoder4Gate>("2x4");

  placementGroup->addGate(gate);

  for (int i = 0; i < 2; ++i) {
    QString iname = Decoder4Gate::iname(i);

    Connection *con = addPlacementConn(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 4; ++i) {
    QString oname = Decoder4Gate::oname(i);

    Connection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
Schematic::
addDecoder8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Decoder8Gate *gate = addGateT<Decoder8Gate>("3x8");

  placementGroup->addGate(gate);

  for (int i = 0; i < 3; ++i) {
    QString iname = Decoder8Gate::iname(i);

    Connection *con = addPlacementConn(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 8; ++i) {
    QString oname = Decoder8Gate::oname(i);

    Connection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
Schematic::
addDecoder16Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder16");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Decoder16Gate *gate = addGateT<Decoder16Gate>("4x16");

  placementGroup->addGate(gate);

  for (int i = 0; i < 4; ++i) {
    QString iname = Decoder16Gate::iname(i);

    Connection *coni = addPlacementConn(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 16; ++i) {
    QString oname = Decoder16Gate::oname(i);

    Connection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
Schematic::
addDecoder256Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_decoder256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Decoder256Gate *gate = addGateT<Decoder256Gate>("8x256");

  placementGroup->addGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString iname = Decoder256Gate::iname(i);

    Connection *coni = addPlacementConn(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 256; ++i) {
    QString oname = Decoder256Gate::oname(i);

    Connection *cono = addPlacementConn(oname);

    gate->connect(oname, cono);
  }
}

void
Schematic::
addLShiftGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_lshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  LShiftGate *gate = addGateT<LShiftGate>("SHL");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *icon[8];
  Connection *ocon[8];

  gate->connect("shift_in" , addPlacementConn("shift_in" ));
  gate->connect("shift_out", addPlacementConn("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(LShiftGate::iname(i));
    ocon[i] = addPlacementConn(LShiftGate::oname(i));

    gate->connect(LShiftGate::iname(i), icon[i]);
    gate->connect(LShiftGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
Schematic::
addRShiftGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_rshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  RShiftGate *gate = addGateT<RShiftGate>("SHR");

  placementGroup->addGate(gate);

  gate->connect("s", addPlacementConn("s"));
  gate->connect("e", addPlacementConn("e"));

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *icon[8];
  Connection *ocon[8];

  gate->connect("shift_in" , addPlacementConn("shift_in" ));
  gate->connect("shift_out", addPlacementConn("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(RShiftGate::iname(i));
    ocon[i] = addPlacementConn(RShiftGate::oname(i));

    gate->connect(RShiftGate::iname(i), icon[i]);
    gate->connect(RShiftGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
Schematic::
addInverterGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  InverterGate *gate = addGateT<InverterGate>("INV");

  placementGroup->addGate(gate);

  Bus *ibus = addPlacementBus("a", 8);
  Bus *obus = addPlacementBus("c", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  Connection *icon[8];
  Connection *ocon[8];

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(InverterGate::iname(i));
    ocon[i] = addPlacementConn(InverterGate::oname(i));

    gate->connect(InverterGate::iname(i), icon[i]);
    gate->connect(InverterGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
Schematic::
addAnderGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_ander");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  AnderGate *gate = addGateT<AnderGate>("AND");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  Connection *acon[8];
  Connection *bcon[8];
  Connection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(AnderGate::aname(i));
    bcon[i] = addPlacementConn(AnderGate::bname(i));
    ccon[i] = addPlacementConn(AnderGate::cname(i));

    gate->connect(AnderGate::aname(i), acon[i]);
    gate->connect(AnderGate::bname(i), bcon[i]);
    gate->connect(AnderGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
Schematic::
addOrerGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_orer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  OrerGate *gate = addGateT<OrerGate>("AND");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  Connection *acon[8];
  Connection *bcon[8];
  Connection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(OrerGate::aname(i));
    bcon[i] = addPlacementConn(OrerGate::bname(i));
    ccon[i] = addPlacementConn(OrerGate::cname(i));

    gate->connect(OrerGate::aname(i), acon[i]);
    gate->connect(OrerGate::bname(i), bcon[i]);
    gate->connect(OrerGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
Schematic::
addXorerGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_xorer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  XorerGate *gate = addGateT<XorerGate>("AND");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  Connection *acon[8];
  Connection *bcon[8];
  Connection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    acon[i] = addPlacementConn(XorerGate::aname(i));
    bcon[i] = addPlacementConn(XorerGate::bname(i));
    ccon[i] = addPlacementConn(XorerGate::cname(i));

    gate->connect(XorerGate::aname(i), acon[i]);
    gate->connect(XorerGate::bname(i), bcon[i]);
    gate->connect(XorerGate::cname(i), ccon[i]);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);
  }
}

void
Schematic::
addAdderGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_adder");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  AdderGate *gate = addGateT<AdderGate>("adder");

  placementGroup->addGate(gate);

  gate->connect("a"        , addPlacementConn("a"        ));
  gate->connect("b"        , addPlacementConn("b"        ));
  gate->connect("carry_in" , addPlacementConn("carry_in" ));
  gate->connect("carry_out", addPlacementConn("carry_out"));
  gate->connect("sum"      , addPlacementConn("sum"      ));
}

void
Schematic::
addAdder8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_adder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Adder8Gate *gate = addGateT<Adder8Gate>("ADD");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString aname = Adder8Gate::aname(i);
    QString bname = Adder8Gate::bname(i);
    QString cname = Adder8Gate::cname(i);

    Connection *acon = addPlacementConn(aname);
    Connection *bcon = addPlacementConn(bname);
    Connection *ccon = addPlacementConn(cname);

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
Schematic::
addComparatorGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_comparator");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ComparatorGate *gate = addGateT<ComparatorGate>("XOR");

  placementGroup->addGate(gate);

  gate->connect("a"        , addPlacementConn("a"       ));
  gate->connect("b"        , addPlacementConn("b"       ));
  gate->connect("c"        , addPlacementConn("c"       ));
  gate->connect("a_larger" , addPlacementConn("a_larger"));
  gate->connect("equal"    , addPlacementConn("equal"   ));
}

void
Schematic::
addComparator8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_comparator8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Comparator8Gate *gate = addGateT<Comparator8Gate>("XOR");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    QString aname = Comparator8Gate::aname(i);
    QString bname = Comparator8Gate::bname(i);
    QString cname = Comparator8Gate::cname(i);

    Connection *acon = addPlacementConn(aname);
    Connection *bcon = addPlacementConn(bname);
    Connection *ccon = addPlacementConn(cname);

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
Schematic::
addBus0Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_bus0");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus0Gate *gate = addGateT<Bus0Gate>("Z");

  placementGroup->addGate(gate);

  Bus *ibus = addPlacementBus("i", 8);

  ibus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    Connection *icon = addPlacementConn(Bus0Gate::iname(i));

    gate->connect(Bus0Gate::iname(i), icon);

    ibus->addConnection(icon, i);
  }

  gate->connect("zero", addPlacementConn("zero"));
}

void
Schematic::
addBus1Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_bus1");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus1Gate *gate = addGateT<Bus1Gate>("BUS1");

  placementGroup->addGate(gate);

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  ibus->setGate(gate);
  obus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    Connection *icon = addPlacementConn(Bus1Gate::iname(i));
    Connection *ocon = addPlacementConn(Bus1Gate::oname(i));

    gate->connect(Bus1Gate::iname(i), icon);
    gate->connect(Bus1Gate::oname(i), ocon);

    ibus->addConnection(icon, i);
    obus->addConnection(ocon, i);
  }

  gate->connect("bus1", addPlacementConn("bus1"));
}

void
Schematic::
addAluGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_alu");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  AluGate *gate = addGateT<AluGate>("ALU");

  placementGroup->addGate(gate);

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  abus->setGate(gate);
  bbus->setGate(gate);
  cbus->setGate(gate);

  for (int i = 0; i < 8; ++i) {
    Connection *acon = addPlacementConn(AluGate::aname(i));
    Connection *bcon = addPlacementConn(AluGate::bname(i));
    Connection *ccon = addPlacementConn(AluGate::cname(i));

    gate->connect(AluGate::aname(i), acon);
    gate->connect(AluGate::bname(i), bcon);
    gate->connect(AluGate::cname(i), ccon);

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
    QString opname = AluGate::opname(i);

    gate->connect(opname, addPlacementConn(opname));
  }
}

void
Schematic::
addClkGate(int delay, int cycle)
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_clk");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  QString name("clk");

  if (delay > 0 || cycle > 0)
    name = QString("clk%1:%2").arg(delay).arg(cycle);

  ClkGate *gate = addGateT<ClkGate>(name);

  gate->setDelay(delay);
  gate->setCycle(cycle);

  placementGroup->addGate(gate);

  Connection *clk = addPlacementConn("clk");

  gate->connect("clk", clk);
}

void
Schematic::
addClkESGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_clk_es");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  QString name("clk");

  ClkESGate *gate = addGateT<ClkESGate>(name);

  placementGroup->addGate(gate);

  Connection *clk  = addPlacementConn("clk"  );
  Connection *clke = addPlacementConn("clk_e");
  Connection *clks = addPlacementConn("clk_s");

  clk ->setTraced(true);
  clke->setTraced(true);
  clks->setTraced(true);

  gate->connect("clk"  , clk);
  gate->connect("clk_e", clke);
  gate->connect("clk_s", clks);
}

void
Schematic::
addStepperGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setExpandName("build_stepper");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  QString name("stepper");

  StepperGate *gate = addGateT<StepperGate>(name);

  placementGroup->addGate(gate);

  Connection *clk  = addPlacementConn("clk"  );
  Connection *con1 = addPlacementConn("1");
  Connection *con2 = addPlacementConn("2");
  Connection *con3 = addPlacementConn("3");
  Connection *con4 = addPlacementConn("4");
  Connection *con5 = addPlacementConn("5");
  Connection *con6 = addPlacementConn("6");
  Connection *con7 = addPlacementConn("7");
  Connection *rcon = addPlacementConn("reset");

  gate->connect("clk"  , clk);
  gate->connect("1"    , con1);
  gate->connect("2"    , con2);
  gate->connect("3"    , con3);
  gate->connect("4"    , con4);
  gate->connect("5"    , con5);
  gate->connect("6"    , con6);
  gate->connect("7"    , con7);
  gate->connect("reset", rcon);
}

//---

void
Schematic::
buildNotGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("not");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NandGate *gate = addGateT<NandGate>("nand");

  placementGroup->addGate(gate);

  Connection *in  = addPlacementConn("a");
  Connection *out = addPlacementConn("c");

  gate->connect("a", in );
  gate->connect("b", in );
  gate->connect("c", out);
}

void
Schematic::
buildAndGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("and");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NandGate *nandGate = addPlacementGateT<NandGate>(placementGroup, "nand", "");
  NotGate  *notGate  = addPlacementGateT<NotGate >(placementGroup, "not" , "build_not" );

  Connection *in1  = addPlacementConn("a");
  Connection *in2  = addPlacementConn("b");
  Connection *out1 = addPlacementConn("x");
  Connection *out2 = addPlacementConn("c");

  nandGate->connect("a", in1 );
  nandGate->connect("b", in2 );
  nandGate->connect("c", out1);

  notGate->connect("a", out1);
  notGate->connect("c", out2);
}

void
Schematic::
buildAnd3Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 2);

  placementGroup->setCollapseName("and3");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  AndGate *andGate1 = addPlacementGateT<AndGate>(placementGroup, "and1", "build_and", 1, 0);
  AndGate *andGate2 = addPlacementGateT<AndGate>(placementGroup, "and2", "build_and", 0, 1);

  Connection *in1  = addPlacementConn("a");
  Connection *in2  = addPlacementConn("b");
  Connection *in3  = addPlacementConn("c");
  Connection *out1 = addPlacementConn("t");
  Connection *out2 = addPlacementConn("d");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);
}

void
Schematic::
buildAnd4Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("and4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  AndGate *andGate1 = addPlacementGateT<AndGate>(placementGroup, "and1", "build_and", 2, 0);
  AndGate *andGate2 = addPlacementGateT<AndGate>(placementGroup, "and2", "build_and", 1, 1);
  AndGate *andGate3 = addPlacementGateT<AndGate>(placementGroup, "and3", "build_and", 0, 2);

  Connection *in1  = addPlacementConn("a");
  Connection *in2  = addPlacementConn("b");
  Connection *in3  = addPlacementConn("c");
  Connection *in4  = addPlacementConn("d");
  Connection *out1 = addPlacementConn("t1");
  Connection *out2 = addPlacementConn("t2");
  Connection *out3 = addPlacementConn("e");

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
Schematic::
buildAnd8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 7, 7);

  placementGroup->setCollapseName("and8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  AndGate *andGate[7];

  for (int i = 0; i < 7; ++i) {
    andGate[i] =
      addPlacementGateT<AndGate>(placementGroup, QString("and%1").arg(i), "build_and", 6 - i, i);
  }

  Connection *in[8];

  for (int i = 0; i < 8; ++i)
    in[i] = addPlacementConn(QString("a%1").arg(i));

  Connection *out[7];

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
Schematic::
buildOrGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 2);

  placementGroup->setCollapseName("or");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NotGate  *notGate1 = addGateT<NotGate>();
  NotGate  *notGate2 = addGateT<NotGate>();
  NandGate *nandGate = addGateT<NandGate>();

  placementGroup->addGate(notGate1, 1, 0);
  placementGroup->addGate(notGate2, 0, 0);
  placementGroup->addGate(nandGate, 0, 1, 2, 1);

  Connection *in1 = addPlacementConn("a");
  Connection *in2 = addPlacementConn("b");
  Connection *io1 = addPlacementConn("c");
  Connection *io2 = addPlacementConn("d");
  Connection *out = addPlacementConn("e");

  notGate1->connect("a", in1);
  notGate1->connect("c", io1);

  notGate2->connect("a", in2);
  notGate2->connect("c", io2);

  nandGate->connect("a", io1);
  nandGate->connect("b", io2);
  nandGate->connect("c", out);
}

void
Schematic::
buildOr8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 7, 7);

  placementGroup->setCollapseName("or8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  OrGate *orGate[7];

  for (int i = 0; i < 7; ++i) {
    orGate[i] = addGateT<OrGate>();

    placementGroup->addGate(orGate[i], 6 - i, i);
  }

  Connection *in[8];

  for (int i = 0; i < 8; ++i)
    in[i] = addPlacementConn(QString("a%1").arg(i));

  Connection *out[7];

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
Schematic::
buildXorGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("xor");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NotGate  *notGate1  = addGateT<NotGate >("not1");
  NotGate  *notGate2  = addGateT<NotGate >("not2");
  NandGate *nandGate1 = addGateT<NandGate>("nand1");
  NandGate *nandGate2 = addGateT<NandGate>("nand2");
  NandGate *nandGate3 = addGateT<NandGate>("nand3");

  placementGroup->addGate(notGate1 , 1, 0);
  placementGroup->addGate(notGate2 , 0, 0);
  placementGroup->addGate(nandGate1, 1, 1);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 0, 2, 2, 1);

  Connection *acon = addPlacementConn("a");
  Connection *bcon = addPlacementConn("b");
  Connection *ccon = addPlacementConn("c");
  Connection *dcon = addPlacementConn("d");
  Connection *econ = addPlacementConn("e");
  Connection *fcon = addPlacementConn("f");
  Connection *gcon = addPlacementConn("g");

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
Schematic::
buildMemoryGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 3);

  placementGroup->setCollapseName("memory");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NandGate *nandGate1 = addGateT<NandGate>("1");
  NandGate *nandGate2 = addGateT<NandGate>("2");
  NandGate *nandGate3 = addGateT<NandGate>("3");
  NandGate *nandGate4 = addGateT<NandGate>("4");

  placementGroup->addGate(nandGate1, 1, 0);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 1, 2);
  placementGroup->addGate(nandGate4, 0, 2);

  Connection *coni = addPlacementConn("i");
  Connection *cons = addPlacementConn("s");
  Connection *cona = addPlacementConn("a");
  Connection *conb = addPlacementConn("b");
  Connection *conc = addPlacementConn("c");
  Connection *cono = addPlacementConn("o");

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
Schematic::
buildMemory8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("memory8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  Connection *cons = addPlacementConn("s");

  MemoryGate *mem[8];

  for (int i = 0; i < 8; ++i) {
    QString memname = QString("mem%1").arg(i);

    mem[i] = addGateT<MemoryGate>(memname);

    mem[i]->setSSide(Side::BOTTOM);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    Connection *coni = addPlacementConn(iname);
    Connection *cono = addPlacementConn(oname);

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
Schematic::
buildEnablerGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("enabler");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *ibus = addPlacementBus("i", 8);
  Bus *obus = addPlacementBus("o", 8);

  Connection *cone = addPlacementConn("e");

  AndGate *gate[8];

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    gate[i] = addGateT<AndGate>(andname);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    Connection *coni = addPlacementConn(iname);
    Connection *cono = addPlacementConn(oname);

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
Schematic::
buildRegisterGate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("register");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Memory8Gate *mem = addGateT<Memory8Gate>("B");
  EnablerGate *ena = addGateT<EnablerGate>("E");

  placementGroup->addGate(mem);
  placementGroup->addGate(ena);

  Connection *cons = addPlacementConn("s");
  Connection *cone = addPlacementConn("e");

  mem->connect("s", cons);
  ena->connect("e", cone);

  Bus *ibus  = addPlacementBus( "i", 8);
  Bus *iobus = addPlacementBus("io", 8);
  Bus *obus  = addPlacementBus( "o", 8);

  for (int i = 0; i < 8; ++i) {
    QString ioname = QString("io%1").arg(i);

    QString iname = Memory8Gate::iname(i);
    QString oname = Memory8Gate::oname(i);

    Connection *icon  = addPlacementConn(iname);
    Connection *iocon = addPlacementConn(ioname);
    Connection *ocon  = addPlacementConn(oname);

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
Schematic::
buildDecoder4Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 5, 3);

  placementGroup->setCollapseName("decoder4");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *cona = addPlacementConn("a");
  Connection *conb = addPlacementConn("b");

  Connection *conna = addPlacementConn("na");
  Connection *connb = addPlacementConn("nb");

  NotGate *notgate[2];

  for (int i = 0; i < 2; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<NotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);

  AndGate *andgate[4];

  for (int i = 0; i < 4; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<AndGate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));

    QString oname = Decoder4Gate::oname(i);

    Connection *out = addPlacementConn(oname);

    andgate[i]->connect("c", out);
  }

  //---

  for (int i = 0; i < 2; ++i)
    placementGroup->addGate(notgate[i], 4 - i, 1 - i);

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(andgate[i], 3 - i, 2);
}

void
Schematic::
buildDecoder8Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 10, 4);

  placementGroup->setCollapseName("decoder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *cona = addPlacementConn("a");
  Connection *conb = addPlacementConn("b");
  Connection *conc = addPlacementConn("c");

  Connection *conna = addPlacementConn("na");
  Connection *connb = addPlacementConn("nb");
  Connection *connc = addPlacementConn("nc");

  NotGate *notgate[3];

  for (int i = 0; i < 3; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<NotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);
  notgate[2]->connect("a", conc);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);
  notgate[2]->connect("c", connc);

  And3Gate *andgate[8];

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<And3Gate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);
    int i3 = (i & 4);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));
    andgate[i]->connect("c", (i3 ? conc : connc));

    QString oname = Decoder8Gate::oname(i);

    Connection *out = addPlacementConn(oname);

    andgate[i]->connect("d", out);
  }

  //---

  for (int i = 0; i < 3; ++i)
    placementGroup->addGate(notgate[i], 9 - i, 2 - i);

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(andgate[i], 8 - i, 3);
}

void
Schematic::
buildDecoder16Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 18, 5);

  placementGroup->setCollapseName("decoder16");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *cona = addPlacementConn("a");
  Connection *conb = addPlacementConn("b");
  Connection *conc = addPlacementConn("c");
  Connection *cond = addPlacementConn("d");

  Connection *conna = addPlacementConn("na");
  Connection *connb = addPlacementConn("nb");
  Connection *connc = addPlacementConn("nc");
  Connection *connd = addPlacementConn("nd");

  NotGate *notgate[4];

  for (int i = 0; i < 4; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<NotGate>(name);
  }

  notgate[0]->connect("a", cona);
  notgate[1]->connect("a", conb);
  notgate[2]->connect("a", conc);
  notgate[3]->connect("a", cond);

  notgate[0]->connect("c", conna);
  notgate[1]->connect("c", connb);
  notgate[2]->connect("c", connc);
  notgate[3]->connect("c", connd);

  And4Gate *andgate[16];

  for (int i = 0; i < 16; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<And4Gate>(andname);

    int i1 = (i & 1);
    int i2 = (i & 2);
    int i3 = (i & 4);
    int i4 = (i & 8);

    andgate[i]->connect("a", (i1 ? cona : conna));
    andgate[i]->connect("b", (i2 ? conb : connb));
    andgate[i]->connect("c", (i3 ? conc : connc));
    andgate[i]->connect("d", (i4 ? cond : connd));

    QString outname = Decoder16Gate::oname(i);

    Connection *out = addPlacementConn(outname);

    andgate[i]->connect("e", out);
  }

  //---

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(notgate[i], 17 - i, 3 - i);

  for (int i = 0; i < 16; ++i)
    placementGroup->addGate(andgate[i], 16 - i, 4);
}

void
Schematic::
buildDecoder256Gate()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 259, 9);

  placementGroup->setCollapseName("decoder256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *cona = addPlacementConn("a");
  Connection *conb = addPlacementConn("b");
  Connection *conc = addPlacementConn("c");
  Connection *cond = addPlacementConn("d");
  Connection *cone = addPlacementConn("e");
  Connection *conf = addPlacementConn("f");
  Connection *cong = addPlacementConn("g");
  Connection *conh = addPlacementConn("h");

  Connection *conna = addPlacementConn("na");
  Connection *connb = addPlacementConn("nb");
  Connection *connc = addPlacementConn("nc");
  Connection *connd = addPlacementConn("nd");
  Connection *conne = addPlacementConn("ne");
  Connection *connf = addPlacementConn("nf");
  Connection *conng = addPlacementConn("ng");
  Connection *connh = addPlacementConn("nh");

  NotGate *notgate[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("not%1").arg(i);

    notgate[i] = addGateT<NotGate>(name);
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

  And8Gate *andgate[256];

  for (int i = 0; i < 256; ++i) {
    QString andname = QString("and%1").arg(i);

    andgate[i] = addGateT<And8Gate>(andname);

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

    QString outname = Decoder256Gate::oname(i);

    Connection *out = addPlacementConn(outname);

    andgate[i]->connect("i", out);
  }

  //---

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(notgate[i], 257 - i, 7 - i);

  for (int i = 0; i < 256; ++i)
    placementGroup->addGate(andgate[i], 256 - i, 8);
}

void
Schematic::
buildLShift()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("lshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  RegisterGate *gate1 = addGateT<RegisterGate>("R1");
  RegisterGate *gate2 = addGateT<RegisterGate>("R2");

  placementGroup->addGate(gate1);
  placementGroup->addGate(gate2);

  Connection *iocon[9];

  iocon[0] = addPlacementConn("shift_out");
  iocon[8] = addPlacementConn("shift_in");

  for (int i = 1; i <= 7; ++i)
    iocon[i] = addPlacementConn(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    Connection *icon1 = addPlacementConn(iname);
    Connection *ocon2 = addPlacementConn(oname);

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
Schematic::
buildRShift()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("rshift");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  RegisterGate *gate1 = addGateT<RegisterGate>("R1");
  RegisterGate *gate2 = addGateT<RegisterGate>("R2");

  placementGroup->addGate(gate1);
  placementGroup->addGate(gate2);

  Connection *iocon[9];

  iocon[0] = addPlacementConn("shift_out");
  iocon[8] = addPlacementConn("shift_in");

  for (int i = 0; i < 7; ++i)
    iocon[i + 1] = addPlacementConn(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    Connection *icon1 = addPlacementConn(iname);
    Connection *ocon2 = addPlacementConn(oname);

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
Schematic::
buildInverter()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("inverter");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *ibus = addPlacementBus("a", 8);
  Bus *obus = addPlacementBus("c", 8);

  NotGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("not%1").arg(i);

    gates[i] = addGateT<NotGate>(name);

    QString iname = QString("a%1").arg(i);
    QString oname = QString("c%1").arg(i);

    Connection *icon = addPlacementConn(iname);
    Connection *ocon = addPlacementConn(oname);

    gates[i]->connect("a", icon);
    gates[i]->connect("c", ocon);

    ibus->addConnection(icon, i);
    obus->addConnection(ocon, i);
  }

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(gates[7 - i]);
}

void
Schematic::
buildAnder()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("ander");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *abus = addPlacementBus("a", 8); abus->setPosition(Bus::Position::START,  0.25);
  Bus *bbus = addPlacementBus("b", 8); bbus->setPosition(Bus::Position::END  , -0.25);
  Bus *cbus = addPlacementBus("c", 8);

  AndGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("and%1").arg(i);

    gates[i] = addGateT<AndGate>(name);

    QString aname = QString("a%1").arg(i);
    QString bname = QString("b%1").arg(i);
    QString cname = QString("c%1").arg(i);

    Connection *acon = addPlacementConn(aname);
    Connection *bcon = addPlacementConn(bname);
    Connection *ccon = addPlacementConn(cname);

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
Schematic::
buildOrer()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("orer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *ibus1 = addPlacementBus("i1", 8); ibus1->setPosition(Bus::Position::START,  0.25);
  Bus *ibus2 = addPlacementBus("i2", 8); ibus2->setPosition(Bus::Position::END  , -0.25);
  Bus *obus  = addPlacementBus("o" , 8);

  OrGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("or%1").arg(i);

    gates[i] = addGateT<OrGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    Connection *icon1 = addPlacementConn(iname1);
    Connection *icon2 = addPlacementConn(iname2);
    Connection *ocon  = addPlacementConn(oname);

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
Schematic::
buildXorer()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("xorer");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *ibus1 = addPlacementBus("i1", 8); ibus1->setPosition(Bus::Position::START,  0.25);
  Bus *ibus2 = addPlacementBus("i2", 8); ibus2->setPosition(Bus::Position::END  , -0.25);
  Bus *obus  = addPlacementBus("o" , 8);

  XorGate *gates[8];

  for (int i = 0; i < 8; ++i) {
    QString name = QString("xor%1").arg(i);

    gates[i] = addGateT<XorGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    Connection *icon1 = addPlacementConn(iname1);
    Connection *icon2 = addPlacementConn(iname2);
    Connection *ocon  = addPlacementConn(oname);

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
Schematic::
buildAdder()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("adder");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  XorGate *xorGate1 = addGateT<XorGate>("xor1");
  XorGate *xorGate2 = addGateT<XorGate>("xor2");
  AndGate *andGate1 = addGateT<AndGate>("and1");
  AndGate *andGate2 = addGateT<AndGate>("and2");
  OrGate  *orGate   = addGateT<OrGate >("or"  );

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(xorGate2, 2, 2);
  placementGroup->addGate(andGate1, 1, 1);
  placementGroup->addGate(andGate2, 0, 1);
  placementGroup->addGate(orGate  , 0, 2, 2, 1);

  Connection *acon  = addPlacementConn("a");
  Connection *bcon  = addPlacementConn("b");
  Connection *cconi = addPlacementConn("carry_in");
  Connection *scon  = addPlacementConn("sum");
  Connection *ccono = addPlacementConn("carry_out");

  Connection *scon1 = addPlacementConn("");
  Connection *ccon1 = addPlacementConn("");
  Connection *ccon2 = addPlacementConn("");

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
Schematic::
buildAdder8()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("adder8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Bus *abus = addPlacementBus("i"  , 8); abus->setPosition(Bus::Position::START,  0.25);
  Bus *bbus = addPlacementBus("o"  , 8); bbus->setPosition(Bus::Position::END  , -0.25);
  Bus *sbus = addPlacementBus("sum", 8);

  AdderGate  *adders[8];
  Connection *acon  [8];
  Connection *bcon  [8];
  Connection *scon  [8];

  Connection *cicon = nullptr;
  Connection *cocon = nullptr;

  for (int i = 0; i < 8; ++i) {
    adders[i] = addGateT<AdderGate>(QString("adder%1").arg(i));

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
Schematic::
buildComparator()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 4, 5);

  placementGroup->setCollapseName("comparator");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  XorGate  *xorGate1 = addGateT<XorGate >("xor1");
  NotGate  *notGate2 = addGateT<NotGate >("not2");
  AndGate  *andGate3 = addGateT<AndGate >("and3");
  And3Gate *andGate4 = addGateT<And3Gate>("and4");
  OrGate   *orGate5  = addGateT<OrGate  >("or5" );

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(notGate2, 1, 1);
  placementGroup->addGate(andGate3, 0, 2);
  placementGroup->addGate(andGate4, 3, 3);
  placementGroup->addGate(orGate5 , 0, 4);

  Connection *acon = addPlacementConn("a");
  Connection *bcon = addPlacementConn("b");
  Connection *ccon = addPlacementConn("c");

  Connection *equalCon     = addPlacementConn("equal");
  Connection *iallEqualCon = addPlacementConn("i_all_equal");
  Connection *oallEqualCon = addPlacementConn("o_all_equal");

  Connection *dcon = addPlacementConn("d");

  Connection *iaLargerCon = addPlacementConn("i_a_larger");
  Connection *oaLargerCon = addPlacementConn("o_a_larger");

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

  andGate3->setOrientation(Gate::Orientation::R90);
  orGate5 ->setOrientation(Gate::Orientation::R90);
}

void
Schematic::
buildComparator8()
{
  PlacementGroup *placementGroup = addPlacementGroup(PlacementGroup::Placement::VERTICAL);

  placementGroup->setCollapseName("comparator8");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Connection *iallEqualCon = addPlacementConn("i_all_equal");
  Connection *iaLargerCon  = addPlacementConn("i_a_larger" );

  iallEqualCon->setValue(1);

  //---

  Bus *abus = addPlacementBus("a", 8); abus->setPosition(Bus::Position::START,  0.25);
  Bus *bbus = addPlacementBus("b", 8); bbus->setPosition(Bus::Position::END  , -0.25);
  Bus *cbus = addPlacementBus("c", 8);

  //---

  Connection *acon[8];
  Connection *bcon[8];
  Connection *ccon[8];

  PlacementGroup *placementGroup1[8];

  for (int i = 0; i < 8; ++i)
    placementGroup1[7 - i] =
      placementGroup->addPlacementGroup(PlacementGroup::Placement::GRID, 4, 5);

  for (int i = 0; i < 8; ++i) {
    XorGate  *xorGate1 = addGateT<XorGate >(QString("xor1[%1]").arg(i));
    NotGate  *notGate2 = addGateT<NotGate >(QString("not2[%1]").arg(i));
    AndGate  *andGate3 = addGateT<AndGate >(QString("and3[%1]").arg(i));
    And3Gate *andGate4 = addGateT<And3Gate>(QString("and4[%1]").arg(i));
    OrGate   *orGate5  = addGateT<OrGate  >(QString("or5[%1]" ).arg(i));

    placementGroup1[i]->addGate(xorGate1, 2, 0);
    placementGroup1[i]->addGate(notGate2, 1, 1);
    placementGroup1[i]->addGate(andGate3, 0, 2);
    placementGroup1[i]->addGate(andGate4, 3, 3);
    placementGroup1[i]->addGate(orGate5 , 0, 4);

    //---

    acon[i] = addPlacementConn("a");
    bcon[i] = addPlacementConn("b");
    ccon[i] = addPlacementConn("c");

    Connection *equalCon = addPlacementConn("equal");

    Connection *dcon = addPlacementConn("d");

    Connection *oallEqualCon = addPlacementConn("o_all_equal");
    Connection *oaLargerCon  = addPlacementConn("o_a_larger" );

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

    andGate3->setOrientation(Gate::Orientation::R90);
    orGate5 ->setOrientation(Gate::Orientation::R90);

    //---

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);

    iallEqualCon = oallEqualCon;
    iaLargerCon  = oaLargerCon;
  }
}

void
Schematic::
buildBus0()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("bus0");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Or8Gate *orGate  = addGateT<Or8Gate>("or");
  NotGate *notGate = addGateT<NotGate>("not");

  Bus *ibus = addPlacementBus("ibus", 8);

  ibus->setGate(orGate);

  Connection *icon[8];

  for (int i = 0; i < 8; ++i) {
    icon[i] = addPlacementConn(Or8Gate::iname(i));

    orGate->connect(Or8Gate::iname(i), icon[i]);

    ibus->addConnection(icon[i], i);
  }

  Connection *ocon = addPlacementConn("o");
  Connection *zcon = addPlacementConn("zero");

  orGate->connect("o", ocon);

  notGate->connect("a", ocon);
  notGate->connect("c", zcon);
}

void
Schematic::
buildBus1()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 8);

  placementGroup->setCollapseName("bus1");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  AndGate *andGate[7];

  for (int i = 0; i < 7; ++i) {
    andGate[i] = addGateT<AndGate>(QString("and%1").arg(i));

    andGate[i]->setOrientation(Gate::Orientation::R90);
  }

  OrGate *orGate = addGateT<OrGate >("or");

  orGate->setOrientation(Gate::Orientation::R90);

  NotGate *notGate = addGateT<NotGate>("not");

  notGate->setOrientation(Gate::Orientation::R180);

  Bus *ibus = addPlacementBus("ibus", 8);
  Bus *obus = addPlacementBus("obus", 8);

  ibus->setFlipped(true);
  obus->setFlipped(true);

  Connection *bcon1 = addPlacementConn("bus1");
  Connection *bcon2 = addPlacementConn("nbus1");

  Connection *icon[8];
  Connection *ocon[8];

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
Schematic::
buildRam256()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 3);

  placementGroup->setCollapseName("ram256");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  RegisterGate *rgate = addGateT<RegisterGate>("R");

  placementGroup->addGate(rgate, 1, 0);

  //--

  Decoder16Gate *hdec = addGateT<Decoder16Gate>("H 4x16");
  Decoder16Gate *vdec = addGateT<Decoder16Gate>("V 4x16");

  placementGroup->addGate(hdec, 1, 2, 1, 1, Alignment::HFILL);
  placementGroup->addGate(vdec, 0, 1, 1, 1, Alignment::VFILL);

  hdec->setOrientation(Gate::Orientation::R90);

  //---

  Bus *ibus  = addPlacementBus("i" , 8);
  Bus *obus1 = addPlacementBus("o1", 4);
  Bus *obus2 = addPlacementBus("o2", 4);

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    Connection *in  = addPlacementConn(iname);
    Connection *out = addPlacementConn(oname);

    rgate->connect(iname, in );
    rgate->connect(oname, out);

    if (i < 4)
      hdec->connect(Decoder16Gate::iname(i    ), out);
    else
      vdec->connect(Decoder16Gate::iname(i - 4), out);

    ibus->addConnection(in , i);

    if (i < 4)
      obus1->addConnection(out, i);
    else
      obus2->addConnection(out, i - 4);
  }

  Connection *hout[16], *vout[16];

  for (int i = 0; i < 16; ++i) {
    QString honame = Decoder16Gate::oname(i);
    QString voname = Decoder16Gate::oname(i);

    hout[i] = addPlacementConn(honame);
    vout[i] = addPlacementConn(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate->connect("s", addPlacementConn("sa"));

  Connection *s = addPlacementConn("s");
  Connection *e = addPlacementConn("e");

  //---

  Connection *bus[8];

  Bus *iobus = addPlacementBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addPlacementConn(QString("bus[%1]").arg(i));

    iobus->addConnection(bus[i], i);
  }

  //---

  PlacementGroup *placementGroup1 =
    placementGroup->addPlacementGroup(PlacementGroup::Placement::GRID, 16, 16, 0, 2);

  for (int r = 0; r < 16; ++r) {
    for (int c = 0; c < 16; ++c) {
      PlacementGroup *placementGroup2 =
        placementGroup1->addPlacementGroup(PlacementGroup::Placement::GRID, 2, 3, 15 - r, c);

      //---

      AndGate *xgate = addGateT<AndGate>(QString("X_%1_%2").arg(r).arg(c));

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      Connection *t1 = addPlacementConn("t1");

      xgate->connect("c", t1);

      AndGate *agate = addGateT<AndGate>(QString("X1_%1_%2").arg(r).arg(c));
      AndGate *bgate = addGateT<AndGate>(QString("X1_%1_%2").arg(r).arg(c));

      RegisterGate *rgate = addGateT<RegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      Connection *t2 = addPlacementConn("t2");
      Connection *t3 = addPlacementConn("t3");

      agate->connect("a", t1);
      agate->connect("b", s);
      agate->connect("c", t2);

      bgate->connect("a", t1);
      bgate->connect("b", e);
      bgate->connect("c", t3);

      rgate->connect("s", t2);
      rgate->connect("e", t3);

      for (int i = 0; i < 8; ++i) {
        QString iname = RegisterGate::iname(i);
        QString oname = RegisterGate::oname(i);

        rgate->connect(iname, bus[i]);
        rgate->connect(oname, bus[i]);
      }

      //---

      placementGroup2->addGate(xgate, 1, 0);
      placementGroup2->addGate(agate, 1, 1);
      placementGroup2->addGate(bgate, 0, 1);
      placementGroup2->addGate(rgate, 0, 2, 2, 1);
    }
  }
}

void
Schematic::
buildRam65536()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 3, 3);

  placementGroup->setCollapseName("ram65536");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  RegisterGate *rgate0 = addGateT<RegisterGate>("R0");
  RegisterGate *rgate1 = addGateT<RegisterGate>("R1");

  placementGroup->addGate(rgate0, 0, 0);
  placementGroup->addGate(rgate1, 2, 2);

  rgate1->setOrientation(Gate::Orientation::R90);

  //--

  Decoder256Gate *hdec = addGateT<Decoder256Gate>("H 8x256");
  Decoder256Gate *vdec = addGateT<Decoder256Gate>("V 8x256");

  placementGroup->addGate(hdec, 1, 2, 1, 1, Alignment::HFILL);
  placementGroup->addGate(vdec, 0, 1, 1, 1, Alignment::VFILL);

  hdec->setOrientation(Gate::Orientation::R90);

  //---

  Bus *ibus  = addPlacementBus("i" , 8);
  Bus *obus1 = addPlacementBus("o1", 8);
  Bus *obus2 = addPlacementBus("o2", 8);

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    Connection *in   = addPlacementConn(iname);
    Connection *out1 = addPlacementConn(oname);
    Connection *out2 = addPlacementConn(oname);

    rgate0->connect(iname, in);
    rgate1->connect(iname, in);

    rgate0->connect(oname, out1);
    rgate1->connect(oname, out2);

    if (i < 4)
      hdec->connect(Decoder256Gate::iname(i), out1);
    else
      vdec->connect(Decoder256Gate::iname(i), out2);

    ibus->addConnection(in, i);

    obus1->addConnection(out1, i);
    obus2->addConnection(out2, i);
  }

  Connection *hout[256], *vout[256];

  for (int i = 0; i < 256; ++i) {
    QString honame = Decoder256Gate::oname(i);
    QString voname = Decoder256Gate::oname(i);

    hout[i] = addPlacementConn(honame);
    vout[i] = addPlacementConn(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate0->connect("s", addPlacementConn("s0"));
  rgate1->connect("s", addPlacementConn("s1"));

  Connection *s = addPlacementConn("s");
  Connection *e = addPlacementConn("e");

  //---

  Connection *bus[8];

  Bus *iobus = addPlacementBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addPlacementConn(QString("bus[%1]").arg(i));

    iobus->addConnection(bus[i], i);
  }

  //---

  PlacementGroup *placementGroup1 =
    placementGroup->addPlacementGroup(PlacementGroup::Placement::GRID, 256, 256, 0, 2);

  for (int r = 0; r < 256; ++r) {
    //std::cerr << "Row: " << r << "\n";

    for (int c = 0; c < 256; ++c) {
      PlacementGroup *placementGroup2 =
        placementGroup1->addPlacementGroup(PlacementGroup::Placement::GRID, 2, 3, 255 - r, c);

      //---

      AndGate *xgate = addGateT<AndGate>(QString("X_%1_%2").arg(r).arg(c));

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      Connection *t1 = addPlacementConn("t1");

      xgate->connect("c", t1);

      AndGate *agate = addGateT<AndGate>(QString("X1_%1_%2").arg(r).arg(c));
      AndGate *bgate = addGateT<AndGate>(QString("X1_%1_%2").arg(r).arg(c));

      RegisterGate *rgate = addGateT<RegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      Connection *t2 = addPlacementConn("t2");
      Connection *t3 = addPlacementConn("t3");

      agate->connect("a", t1);
      agate->connect("b", s);
      agate->connect("c", t2);

      bgate->connect("a", t1);
      bgate->connect("b", e);
      bgate->connect("c", t3);

      rgate->connect("s", t2);
      rgate->connect("e", t3);

      for (int i = 0; i < 8; ++i) {
        QString iname = RegisterGate::iname(i);
        QString oname = RegisterGate::oname(i);

        rgate->connect(iname, bus[i]);
        rgate->connect(oname, bus[i]);
      }

      //---

      placementGroup2->addGate(xgate, 0, 0);
      placementGroup2->addGate(agate, 0, 1);
      placementGroup2->addGate(bgate, 1, 1);
      placementGroup2->addGate(rgate, 1, 2, 2, 1);
    }
  }
}

void
Schematic::
buildAlu()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 8, 4);

  placementGroup->setCollapseName("alu");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  auto addPlacementBus = [&](const QString &name, int n) {
    Bus *bus = addBus(name, n);

    placementGroup->addBus(bus);

    return bus;
  };

  //---

  Comparator8Gate *xorerGate   = addGateT<Comparator8Gate>("XOR");
  OrerGate        *orerGate    = addGateT<OrerGate       >("OR" );
  AnderGate       *anderGate   = addGateT<AnderGate      >("AND");
  InverterGate    *noterGate   = addGateT<InverterGate   >("NOT");
  LShiftGate      *lshiftGate  = addGateT<LShiftGate     >("SHL");
  RShiftGate      *rshiftGate  = addGateT<RShiftGate     >("SHR");
  Adder8Gate      *adderGate   = addGateT<Adder8Gate     >("ADD");
  Decoder8Gate    *decoderGate = addGateT<Decoder8Gate   >("3x8");

  Gate *gates[7];

  gates[0] = xorerGate;
  gates[1] = orerGate;
  gates[2] = anderGate;
  gates[3] = noterGate;
  gates[4] = lshiftGate;
  gates[5] = rshiftGate;
  gates[6] = adderGate;

  EnablerGate *enablerGate[7];

  for (int i = 0; i < 7; ++i)
    enablerGate[i] = addGateT<EnablerGate>("E");

  AndGate *andGate[3];

  for (int i = 0; i < 3; ++i)
    andGate[i] = addGateT<AndGate>("and");

  Bus0Gate *zeroGate = addGateT<Bus0Gate>("Z");

  placementGroup->addGate(xorerGate  , 7, 0);
  placementGroup->addGate(orerGate   , 6, 0);
  placementGroup->addGate(anderGate  , 5, 0);
  placementGroup->addGate(noterGate  , 4, 0);
  placementGroup->addGate(lshiftGate , 3, 0);
  placementGroup->addGate(rshiftGate , 2, 0);
  placementGroup->addGate(adderGate  , 1, 0);
  placementGroup->addGate(decoderGate, 0, 1);

  for (int i = 0; i < 3; ++i) {
    PlacementGroup *placementGroup1 =
      placementGroup->addPlacementGroup(PlacementGroup::Placement::VERTICAL, 1, 1, i + 1, 2);

    placementGroup1->addGate(enablerGate[i]);
    placementGroup1->addGate(andGate    [i]);
  }

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(enablerGate[3 + i], 4 + i, 2);

  placementGroup->addGate(zeroGate, 6, 3, 2, 1);

  //---

  Bus *abus = addPlacementBus("a", 8);
  Bus *bbus = addPlacementBus("b", 8);
  Bus *cbus = addPlacementBus("c", 8);

  Bus *cbus1[7];

  for (int i = 0; i < 7; ++i) {
    cbus1[i] = addPlacementBus("c", 8);

    cbus1[i]->setGate(gates[i]);
  }

  //---

  Connection *acon[8];
  Connection *bcon[8];
  Connection *ccon[8];
  Connection *ccon1[7][8];

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

    auto connectABC = [&](Gate *gate, int ig) {
      gate->connect(aname, acon[i]);
      gate->connect(bname, bcon[i]);
      gate->connect(cname, ccon1[ig][i]);
    };

    connectABC(xorerGate, 0);
    connectABC(orerGate , 1);
    connectABC(anderGate, 2);

    noterGate->connect(aname, acon[i]);
    noterGate->connect(cname, ccon1[3][i]);

    lshiftGate->connect(LShiftGate::iname(i), acon[i]);
    lshiftGate->connect(LShiftGate::oname(i), ccon1[4][i]);

    rshiftGate->connect(RShiftGate::iname(i), acon[i]);
    rshiftGate->connect(RShiftGate::oname(i), ccon1[5][i]);

    connectABC(adderGate, 6);
  }

  for (int j = 0; j < 7; ++j) {
    for (int i = 0; i < 8; ++i) {
      enablerGate[j]->connect(EnablerGate::iname(i), ccon1[j][i]);
      enablerGate[j]->connect(EnablerGate::oname(i), ccon[i]);
    }
  }

  xorerGate->connect("a_larger", addPlacementConn("a_larger"));
  xorerGate->connect("equal"   , addPlacementConn("equal"   ));

  for (int i = 0; i < 8; ++i)
    zeroGate->connect(Bus0Gate::iname(i), ccon[i]);

  zeroGate->connect("zero", addPlacementConn("zero"));

  decoderGate->connect(Decoder8Gate::iname(0), addPlacementConn("d1"));
  decoderGate->connect(Decoder8Gate::iname(1), addPlacementConn("d2"));
  decoderGate->connect(Decoder8Gate::iname(2), addPlacementConn("d3"));

  Connection *econ[8];

  for (int i = 0; i < 8; ++i) {
    econ[i] = addPlacementConn("e");

    decoderGate->connect(Decoder8Gate::oname(i), econ[i]);

    if (i > 0)
      enablerGate[i - 1]->connect("e", econ[i]);
  }

  Connection *carry_out1 = addPlacementConn("carry_out");

  Connection aocon[3];

  for (int i = 0; i < 3; ++i)
    andGate[i]->connect("c", carry_out1);

  Connection *lshift_out = addPlacementConn("shift_out");
  Connection *rshift_out = addPlacementConn("shift_out");
  Connection *carry_out  = addPlacementConn("carry_out");

  andGate[0]->connect("a", lshift_out);
  andGate[0]->connect("b", econ[5]);

  andGate[1]->connect("a", rshift_out);
  andGate[1]->connect("b", econ[6]);

  andGate[2]->connect("a", carry_out);
  andGate[2]->connect("b", econ[7]);
}

void
Schematic::
buildClk()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::HORIZONTAL);

  placementGroup->setCollapseName("clk");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  NotGate *notGate = addGateT<NotGate>("CLK");

  placementGroup->addGate(notGate);

  Connection *clk = addPlacementConn("clk");

  notGate->connect("a", clk);
  notGate->connect("c", clk);
}

void
Schematic::
buildClkES()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 8, 4);

  placementGroup->setCollapseName("clk");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ClkGate *clkGate  = addGateT<ClkGate>("clk");
  ClkGate *clkdGate = addGateT<ClkGate>("clk_e");
  OrGate  *orGate   = addGateT<OrGate >("or");
  AndGate *andGate  = addGateT<AndGate>("and");

  clkGate ->setDelay(0); clkGate ->setCycle(8);
  clkdGate->setDelay(4); clkdGate->setCycle(8);

  placementGroup->addGate(clkGate , 1, 0);
  placementGroup->addGate(clkdGate, 0, 0);
  placementGroup->addGate(andGate , 1, 1);
  placementGroup->addGate(orGate  , 0, 1);

  Connection *conc = addPlacementConn("clk");
  Connection *cond = addPlacementConn("clk_d");
  Connection *cone = addPlacementConn("clk_e");
  Connection *cons = addPlacementConn("clk_s");

  conc->setTraced(true);
  cond->setTraced(true);
  cone->setTraced(true);
  cons->setTraced(true);

  clkGate ->connect("clk", conc);
  clkdGate->connect("clk", cond);

  orGate->connect("a", conc);
  orGate->connect("b", cond);
  orGate->connect("c", cone);

  andGate->connect("a", conc);
  andGate->connect("b", cond);
  andGate->connect("c", cons);
}

void
Schematic::
buildStepper()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 5, 15);

  placementGroup->setCollapseName("stepper");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *rcon  = addPlacementConn("reset");
  Connection *nrcon = addPlacementConn("nreset");

  NotGate *resetNotGate = addGateT<NotGate>("reset");

  placementGroup->addGate(resetNotGate, 2, 2);

  resetNotGate->connect("a", rcon);
  resetNotGate->connect("c", nrcon);

  //---

  Connection *ccon  = addPlacementConn("clk");
  Connection *nccon = addPlacementConn("not_clk");

  ClkGate *clk = addGateT<ClkGate>("clk");

  placementGroup->addGate(clk, 1, 0);

  clk->connect("clk", ccon);

  NotGate *clkNotGate = addGateT<NotGate>("not");

  placementGroup->addGate(clkNotGate, 1, 1);

  clkNotGate->connect("a", ccon);
  clkNotGate->connect("c", nccon);

  //---

  OrGate *orGate1 = addGateT<OrGate>("or1");
  OrGate *orGate2 = addGateT<OrGate>("or2");

  placementGroup->addGate(orGate1, 0, 2);
  placementGroup->addGate(orGate2, 1, 2);

  Connection *ocon1 = addPlacementConn("or1");
  Connection *ocon2 = addPlacementConn("or2");

  orGate1->connect("a", ccon );
  orGate1->connect("b", ccon );
  orGate1->connect("c", ocon1);

  orGate2->connect("a", rcon );
  orGate2->connect("b", nccon);
  orGate2->connect("c", ocon2);

  MemoryGate *mem[12];

  for (int i = 0; i < 12; ++i) {
    mem[i] = addGateT<MemoryGate>(QString("mem%1").arg(i));

    placementGroup->addGate(mem[i], 2, i + 3);
  }

  Connection *ocon = addPlacementConn("on");

  Connection *coni[11];

  for (int i = 0; i < 12; ++i) {
    if (i == 0) {
      mem[i]->connect("i", nrcon);
    }
    else {
      coni[i - 1] = addPlacementConn(QString("coni%1").arg(i - 1));

      mem[i - 1]->connect("o", coni[i - 1]);
      mem[i    ]->connect("i", coni[i - 1]);
    }

    if (i & 1)
      mem[i]->connect("s", ocon2);
    else
      mem[i]->connect("s", ocon1);
  }

  //---

  NotGate *notGate[6];

  Connection *nscon[6];

  for (int i = 0; i < 6; ++i) {
    notGate[i] = addGateT<NotGate>(QString("not%1").arg(i));

    placementGroup->addGate(notGate[i], 3, i*2 + 4);

    notGate[i]->setOrientation(Gate::Orientation::R180);

    if (i < 5)
      notGate[i]->connect("a", coni[2*i + 1]);

    nscon[i] = addPlacementConn(QString("nscon%1").arg(i));

    notGate[i]->connect("c", nscon[i]);
  }

  OrGate *orGate3 = addGateT<OrGate>("or3");

  placementGroup->addGate(orGate3, 4, 3);

  orGate3->setOrientation(Gate::Orientation::R270);

  Connection *scon[6];

  for (int i = 0; i < 7; ++i)
    scon[i] = addPlacementConn(QString("scon%1").arg(i));

  orGate3->connect("a", rcon);
  orGate3->connect("b", nscon[0]);
  orGate3->connect("c", scon[0]);

  AndGate *andGate[5];

  for (int i = 0; i < 5; ++i) {
    andGate[i] = addGateT<AndGate>(QString("and%1").arg(i));

    andGate[i]->setOrientation(Gate::Orientation::R270);

    placementGroup->addGate(andGate[i], 4, 2*i + 5);

    andGate[i]->connect("a", coni[2*i + 1]);
    andGate[i]->connect("b", nscon[i + 1]);
    andGate[i]->connect("c", scon[i + 1]);
  }

  mem    [11]->connect("o", rcon);
  notGate[5 ]->connect("a", rcon);

  ocon->setValue(true);
}

void
Schematic::
buildControl1()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 9, 8);

  placementGroup->setCollapseName("control1");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ClkESGate   *clkGate = addGateT<ClkESGate  >("CLK");
  StepperGate *stepper = addGateT<StepperGate>("stepper");

  stepper->setOrientation(Gate::Orientation::R90);
  stepper->setFlipped(true);

  Connection *ccon  = addPlacementConn("clk");
  Connection *ccons = addPlacementConn("clk_s");
  Connection *ccone = addPlacementConn("clk_e");

  clkGate->connect("clk"  , ccon);
  clkGate->connect("clk_s", ccons);
  clkGate->connect("clk_e", ccone);

  Connection *rcon = addPlacementConn("reset");

  stepper->connect("clk"  , ccon);
  stepper->connect("reset", rcon);
  stepper->connect("7"    , rcon);

  Connection *scon[6];

  for (int i = 1; i <= 6; ++i) {
    QString cname = QString("%1").arg(i);

    scon[i - 1] = addPlacementConn(cname);

    stepper->connect(cname, scon[i - 1]);
  }

  placementGroup->addGate(clkGate, 8, 0);
  placementGroup->addGate(stepper, 8, 1, 1, 6, Alignment::HFILL);

  AndGate    *land[6];
  Connection *licon[6];
  Connection *locon[6];

  QStringList lnames = QStringList() <<
   "RAM" << "ACC" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 6; ++i) {
    land[i] = addGateT<AndGate>(lnames[i]);

    placementGroup->addGate(land[i], 5 - i, 0);

    land[i]->setOrientation(Gate::Orientation::R180);

    licon[i] = addPlacementConn("");
    locon[i] = addPlacementConn(lnames[i]);

    land[i]->connect("a", ccone);
    land[i]->connect("b", licon[i]);
    land[i]->connect("c", locon[i]);
  }

  AndGate    *rand[8];
  Connection *ricon[8];
  Connection *rocon[8];

  QStringList rnames = QStringList() <<
   "MAR" << "ACC" << "RAM" << "TMP" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 8; ++i) {
    rand[i] = addGateT<AndGate>(rnames[i]);

    placementGroup->addGate(rand[i], 7 - i, 7);

    ricon[i] = addPlacementConn("");
    rocon[i] = addPlacementConn(rnames[i]);

    rand[i]->connect("a", ccons);
    rand[i]->connect("b", ricon[i]);
    rand[i]->connect("c", rocon[i]);
  }
}

void
Schematic::
buildControl2()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 9, 8);

  placementGroup->setCollapseName("control2");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ClkESGate   *clkGate = addGateT<ClkESGate  >("CLK");
  StepperGate *stepper = addGateT<StepperGate>("stepper");

  stepper->setOrientation(Gate::Orientation::R90);
  stepper->setFlipped(true);

  Connection *ccon  = addPlacementConn("clk");
  Connection *ccons = addPlacementConn("clk_s");
  Connection *ccone = addPlacementConn("clk_e");

  clkGate->connect("clk"  , ccon);
  clkGate->connect("clk_s", ccons);
  clkGate->connect("clk_e", ccone);

  Connection *rcon = addPlacementConn("reset");

  stepper->connect("clk"  , ccon);
  stepper->connect("reset", rcon);
  stepper->connect("7"    , rcon);

  Connection *scon[7];

  for (int i = 1; i <= 6; ++i) {
    QString cname = QString("%1").arg(i);

    scon[i] = addPlacementConn(cname);

    stepper->connect(cname, scon[i]);
  }

  placementGroup->addGate(clkGate, 8, 0);
  placementGroup->addGate(stepper, 8, 1, 1, 6, Alignment::HFILL);

  AndGate    *land[6];
  Connection *licon[6];
  Connection *locon[6];

  QStringList lnames = QStringList() <<
   "RAM" << "ACC" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 6; ++i) {
    land[i] = addGateT<AndGate>(lnames[i]);

    placementGroup->addGate(land[i], 5 - i, 0);

    land[i]->setOrientation(Gate::Orientation::R180);

    licon[i] = addPlacementConn("");
    locon[i] = addPlacementConn(lnames[i]);

    land[i]->connect("a", ccone);
    land[i]->connect("b", licon[i]);
    land[i]->connect("c", locon[i]);
  }

  land[1]->connect("b", scon[6]);
  land[2]->connect("b", scon[5]);
  land[3]->connect("b", scon[4]);

  AndGate    *rand[8];
  Connection *ricon[8];
  Connection *rocon[8];

  QStringList rnames = QStringList() <<
   "MAR" << "ACC" << "RAM" << "TMP" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 8; ++i) {
    rand[i] = addGateT<AndGate>(rnames[i]);

    placementGroup->addGate(rand[i], 7 - i, 7);

    ricon[i] = addPlacementConn("");
    rocon[i] = addPlacementConn(rnames[i]);

    rand[i]->connect("a", ccons);
    rand[i]->connect("b", ricon[i]);
    rand[i]->connect("c", rocon[i]);
  }

  rand[1]->connect("b", scon[5]);
  rand[3]->connect("b", scon[4]);
  rand[4]->connect("b", scon[6]);

  scon [4]->setTraced(true);
  locon[3]->setTraced(true);
  rocon[3]->setTraced(true);
  scon [5]->setTraced(true);
  locon[2]->setTraced(true);
  rocon[1]->setTraced(true);
  scon [6]->setTraced(true);
  locon[1]->setTraced(true);
  rocon[4]->setTraced(true);
}

void
Schematic::
buildControl3()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 9, 8);

  placementGroup->setCollapseName("control2");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ClkESGate   *clkGate = addGateT<ClkESGate  >("CLK");
  StepperGate *stepper = addGateT<StepperGate>("stepper");

  stepper->setOrientation(Gate::Orientation::R90);
  stepper->setFlipped(true);

  Connection *ccon  = addPlacementConn("clk");
  Connection *ccons = addPlacementConn("clk_s");
  Connection *ccone = addPlacementConn("clk_e");

  clkGate->connect("clk"  , ccon);
  clkGate->connect("clk_s", ccons);
  clkGate->connect("clk_e", ccone);

  Connection *rcon = addPlacementConn("reset");

  stepper->connect("clk"  , ccon);
  stepper->connect("reset", rcon);
  stepper->connect("7"    , rcon);

  Connection *scon[7];

  for (int i = 1; i <= 6; ++i) {
    QString cname = QString("%1").arg(i);

    scon[i] = addPlacementConn(cname);

    stepper->connect(cname, scon[i]);
  }

  placementGroup->addGate(clkGate, 8, 0);
  placementGroup->addGate(stepper, 8, 1, 1, 6, Alignment::HFILL);

  AndGate    *land[6];
  Connection *licon[6];
  Connection *locon[6];

  QStringList lnames = QStringList() <<
   "RAM" << "ACC" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 6; ++i) {
    land[i] = addGateT<AndGate>(lnames[i]);

    placementGroup->addGate(land[i], 5 - i, 0);

    land[i]->setOrientation(Gate::Orientation::R180);

    licon[i] = addPlacementConn("");
    locon[i] = addPlacementConn(lnames[i]);

    land[i]->connect("a", ccone);
    land[i]->connect("b", licon[i]);
    land[i]->connect("c", locon[i]);
  }

  land[2]->connect("b", scon[5]);
  land[4]->connect("b", scon[4]);

  AndGate    *rand[8];
  Connection *ricon[8];
  Connection *rocon[8];

  QStringList rnames = QStringList() <<
   "MAR" << "ACC" << "RAM" << "TMP" << "R0" << "R1" << "R2" << "R3";

  for (int i = 0; i < 8; ++i) {
    rand[i] = addGateT<AndGate>(rnames[i]);

    placementGroup->addGate(rand[i], 7 - i, 7);

    ricon[i] = addPlacementConn("");
    rocon[i] = addPlacementConn(rnames[i]);

    rand[i]->connect("a", ccons);
    rand[i]->connect("b", ricon[i]);
    rand[i]->connect("c", rocon[i]);
  }

  rand[0]->connect("b", scon[4]);
  rand[2]->connect("b", scon[5]);
}

void
Schematic::
buildControl4()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 10, 8);

  placementGroup->setCollapseName("control2");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  ClkESGate   *clkGate = addGateT<ClkESGate  >("CLK");
  StepperGate *stepper = addGateT<StepperGate>("stepper");

  stepper->setOrientation(Gate::Orientation::R90);
  stepper->setFlipped(true);

  Connection *ccon  = addPlacementConn("clk");
  Connection *ccons = addPlacementConn("clk_s");
  Connection *ccone = addPlacementConn("clk_e");

  clkGate->connect("clk"  , ccon);
  clkGate->connect("clk_s", ccons);
  clkGate->connect("clk_e", ccone);

  Connection *rcon = addPlacementConn("reset");

  stepper->connect("clk"  , ccon);
  stepper->connect("reset", rcon);
  stepper->connect("7"    , rcon);

  Connection *scon[7];

  for (int i = 1; i <= 6; ++i) {
    QString cname = QString("%1").arg(i);

    scon[i] = addPlacementConn(cname);

    stepper->connect(cname, scon[i]);
  }

  placementGroup->addGate(clkGate, 9, 0);
  placementGroup->addGate(stepper, 9, 1, 1, 6, Alignment::HFILL);

  Bus1Gate *bus1 = addGateT<Bus1Gate>("bus1");

  placementGroup->addGate(bus1, 4, 0);

  AndGate    *land[6];
  Connection *licon[6];
  Connection *locon[6];

  QStringList lnames = QStringList() <<
   "IAR" << "RAM" << "ACC";

  for (int i = 0; i < 3; ++i) {
    land[i] = addGateT<AndGate>(lnames[i]);

    placementGroup->addGate(land[i], 3 - i, 0);

    land[i]->setOrientation(Gate::Orientation::R180);

    licon[i] = addPlacementConn("");
    locon[i] = addPlacementConn(lnames[i]);

    land[i]->connect("a", ccone);
    land[i]->connect("b", licon[i]);
    land[i]->connect("c", locon[i]);
  }

  land[0]->connect("b", scon[1]);
  land[1]->connect("b", scon[2]);
  land[2]->connect("b", scon[3]);

  bus1->connect("bus1", scon[1]);

  AndGate    *rand[8];
  Connection *ricon[8];
  Connection *rocon[8];

  QStringList rnames = QStringList() <<
   "IR" << "MAR" << "IAR" << "ACC";

  for (int i = 0; i < 4; ++i) {
    rand[i] = addGateT<AndGate>(rnames[i]);

    placementGroup->addGate(rand[i], 4 - i, 7);

    ricon[i] = addPlacementConn("");
    rocon[i] = addPlacementConn(rnames[i]);

    rand[i]->connect("a", ccons);
    rand[i]->connect("b", ricon[i]);
    rand[i]->connect("c", rocon[i]);
  }

  rand[0]->connect("b", scon[2]);
  rand[1]->connect("b", scon[1]);
  rand[2]->connect("b", scon[3]);
  rand[3]->connect("b", scon[1]);

  RegisterGate *ir = addGateT<RegisterGate>("IR");

  placementGroup->addGate(ir, 0, 4, 1, 3, Alignment::HFILL);

  ir->connect("s", rocon[0]);
}

void
Schematic::
buildControl5()
{
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 9, 6);

  placementGroup->setCollapseName("control2");

  //---

  auto addPlacementConn = [&](const QString &name) {
    Connection *conn = addConnection(name);

    placementGroup->addConnection(conn);

    return conn;
  };

  //---

  Connection *ccone = addPlacementConn("clk_e");
  Connection *ccons = addPlacementConn("clk_s");

  Connection *clrega = addPlacementConn("reg_a");
  Connection *clregb = addPlacementConn("reg_b");

  Connection *crregb = addPlacementConn("reg_b");

  Decoder4Gate *lgate1 = addGateT<Decoder4Gate>("2x4 H");
  Decoder4Gate *lgate2 = addGateT<Decoder4Gate>("2x4 L");
  Decoder4Gate *rgate  = addGateT<Decoder4Gate>("2x4");

  lgate1->setOrientation(Gate::Orientation::R180);
  lgate2->setOrientation(Gate::Orientation::R180);

  placementGroup->addGate(lgate1, 5, 2, 4, 1, Alignment::VFILL);
  placementGroup->addGate(lgate2, 1, 2, 4, 1, Alignment::VFILL);
  placementGroup->addGate(rgate , 5, 4, 4, 1, Alignment::VFILL);

  Connection *lgate1ca = addPlacementConn("");
  Connection *lgate1cb = addPlacementConn("");

  Connection *lgate2ca = addPlacementConn("");
  Connection *lgate2cb = addPlacementConn("");

  lgate1->connect("a", lgate1ca);
  lgate1->connect("b", lgate1cb);
  lgate2->connect("a", lgate2ca);
  lgate2->connect("b", lgate2cb);

  rgate->connect("a", lgate1ca);
  rgate->connect("b", lgate1cb);

  And3Gate   *land[8];
  Connection *landcc[8];
  Connection *landcd[8];

  for (int i = 0; i < 8; ++i) {
    land[i] = addGateT<And3Gate>(QString("land%1").arg(i));

    land[i]->setOrientation(Gate::Orientation::R180);

    placementGroup->addGate(land[i], 8 - i, 1);

    landcc[i] = addPlacementConn("");
    landcd[i] = addPlacementConn("");

    land[i]->connect("a", ccone);

    if (i < 4)
      land[i]->connect("b", clregb);
    else
      land[i]->connect("b", clrega);

    land[i]->connect("c", landcc[i]);
    land[i]->connect("d", landcd[i]);
  }

  for (int i = 0; i < 4; ++i) {
    lgate1->connect(lgate1->oname(i), landcc[i]);
    lgate2->connect(lgate2->oname(i), landcc[4 + i]);
  }

  OrGate     *lor[4];
  Connection *lorcc[4];

  for (int i = 0; i < 4; ++i) {
    lor[i] = addGateT<OrGate>(QString("lor%1").arg(i));

    lor[i]->setOrientation(Gate::Orientation::R180);

    placementGroup->addGate(lor[i], 8 - i, 0);

    lorcc[i] = addPlacementConn("");

    lor[i]->connect("a", landcd[i]);
    lor[i]->connect("b", landcd[4 + i]);
    lor[i]->connect("c", lorcc[i]);
  }

  And3Gate   *rand[8];
  Connection *rconc[8];
  Connection *rcond[8];

  for (int i = 0; i < 4; ++i) {
    rand[i] = addGateT<And3Gate>(QString("rand%1").arg(i));

    placementGroup->addGate(rand[i], 8 - i, 5);

    rconc[i] = addPlacementConn("");
    rcond[i] = addPlacementConn("");

    rand[i]->connect("a", ccons);
    rand[i]->connect("b", crregb);
    rand[i]->connect("c", rconc[i]);
    rand[i]->connect("d", rcond[i]);

    rgate->connect(rgate->oname(i), rconc[i]);
  }

  RegisterGate *ir = addGateT<RegisterGate>("IR");

  ir->setOrientation(Gate::Orientation::R270);

  ir->connect(ir->oname(7), lgate1ca);
  ir->connect(ir->oname(6), lgate1cb);

  ir->connect(ir->oname(5), lgate2ca);
  ir->connect(ir->oname(4), lgate2cb);

  placementGroup->addGate(ir, 0, 3, 1, 1, Alignment::HFILL);
}

void
Schematic::
testConnection()
{
#if 0
  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 2);

  NotGate *notGate1 = addGateT<NotGate>("N1");
  NotGate *notGate2 = addGateT<NotGate>("N2");
  NotGate *notGate3 = addGateT<NotGate>("N3");
  NotGate *notGate4 = addGateT<NotGate>("N4");

  Connection *conn1 = addConnection("c1");

  notGate1->connect("c", conn1);
  notGate2->connect("c", conn1);
  notGate3->connect("a", conn1);
  notGate4->connect("a", conn1);

  placementGroup->addGate(notGate1, 1, 0);
  placementGroup->addGate(notGate2, 0, 0);
  placementGroup->addGate(notGate3, 1, 1);
  placementGroup->addGate(notGate4, 0, 1);
#endif

  PlacementGroup *placementGroup =
    addPlacementGroup(PlacementGroup::Placement::GRID, 2, 1);

  NotGate *notGate1 = addGateT<NotGate>("N1");
  NotGate *notGate2 = addGateT<NotGate>("N2");

  Connection *conn1 = addConnection("c1");

  notGate1->connect("c", conn1);
  notGate2->connect("a", conn1);

  placementGroup->addGate(notGate1, 1, 0);
  placementGroup->addGate(notGate2, 0, 0);
}

//------

Connection *
Schematic::
addConnection(const QString &name)
{
  Connection *connection = new Connection(name);

  connection->setSchem(this);

  connections_.push_back(connection);

  return connection;
}

void
Schematic::
removeConnection(Connection *connection)
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
Schematic::
addGate(Gate *gate)
{
  gates_.push_back(gate);

  placementGroup_->addGate(gate);
}

void
Schematic::
removeGate(Gate *gate)
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

Bus *
Schematic::
addBus(const QString &name, int n)
{
  Bus *bus = new Bus(name, n);

  buses_.push_back(bus);

  return bus;
}

void
Schematic::
removeBus(Bus *bus)
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

PlacementGroup *
Schematic::
addPlacementGroup(PlacementGroup::Placement placement, int nr, int nc)
{
  PlacementGroup *placementGroup = new PlacementGroup(placement, nr, nc);

  addPlacementGroup(placementGroup);

  return placementGroup;
}

void
Schematic::
addPlacementGroup(PlacementGroup *placementGroup)
{
  placementGroup_->addPlacementGroup(placementGroup);
}

bool
Schematic::
exec()
{
  bool changed = false;

  for (auto &gate : gates_) {
    if (gate->exec())
      changed = true;
  }

  ++t_;

  redraw();

  return changed;
}

void
Schematic::
test()
{
  std::vector<Connection *> in, out;

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
Schematic::
addTValue(const Connection *connection, bool b)
{
  if (waveform_) {
    waveform_->addValue(connection, t_, b);

    waveform_->update();
  }
}

void
Schematic::
place()
{
  placementGroup_->place();

  calcBounds();
}

void
Schematic::
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
Schematic::
resizeEvent(QResizeEvent *)
{
  renderer_.displayRange.setEqualScale(true);

  renderer_.displayRange.setPixelRange(0, 0, this->width() - 1, this->height() - 1);

  image_   = QImage(size(), QImage::Format_ARGB32);
  changed_ = true;

  calcBounds();
}

void
Schematic::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  if (changed_) {
    // draw new data
    QPainter ipainter(&image_);

    ipainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    ipainter.fillRect(rect(), QBrush(Qt::black));

    renderer_.schem         = this;
    renderer_.painter       = &ipainter;
    renderer_.prect         = rect();
    renderer_.rect          = rect_;
    renderer_.placementRect = placementGroup_->rect();
    renderer_.selected      = false;
    renderer_.inside        = false;

    for (const auto &gate : gates_)
      gate->draw(&renderer_);

    for (const auto &connection : connections_) {
      if (! connection->bus())
        connection->draw(&renderer_);
    }

    for (const auto &bus : buses_)
      bus->draw(&renderer_);

    placementGroup_->draw(&renderer_);

    changed_ = false;
  }

  //---

  // draw cached data
  painter.drawImage(0, 0, image_);

  //---

  // draw selected and inside
  renderer_.schem         = this;
  renderer_.painter       = &painter;
  renderer_.prect         = rect();
  renderer_.rect          = rect_;
  renderer_.placementRect = placementGroup_->rect();

  Gates selGates;

  selectedGates(selGates);

  for (const auto &gate : selGates) {
    renderer_.selected = true;
    renderer_.inside   = false;

    gate->draw(&renderer_);
  }

  if (insideGate()) {
    renderer_.selected = false;
    renderer_.inside   = true;

    insideGate()->draw(&renderer_);
  }

  Connections selConnections;

  selectedConnections(selConnections);

  for (const auto &connection : selConnections) {
    renderer_.selected = true;
    renderer_.inside   = false;

    connection->draw(&renderer_);
  }

  if (insideConnection()) {
    renderer_.selected = false;
    renderer_.inside   = true;

    insideConnection()->draw(&renderer_);
  }

#if 0
  Buses selBuses;

  selectedBuses(selBuses);

  for (const auto &bus : selBuses) {
    renderer_.selected = true;
    renderer_.inside   = false;

    bus->draw(&renderer_);
  }
#endif

  PlacementGroups selPlacementGroups;

  selectedPlacementGroups(selPlacementGroups);

  for (const auto &placementGroup : selPlacementGroups) {
    renderer_.selected = true;
    renderer_.inside   = false;

    placementGroup->draw(&renderer_);
  }

  if (insidePlacement()) {
    renderer_.selected = false;
    renderer_.inside   = true;

    insidePlacement()->draw(&renderer_);
  }
}

void
Schematic::
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
        deselectAll();

        pressGate_->setSelected(! pressGate_->isSelected());

        update();
      }
    }
    else if (movePlacement_) {
      pressPlacement_ = nearestPlacementGroup(pressPoint_);

      if (pressPlacement_) {
        deselectAll();

        pressPlacement_->setSelected(! pressPlacement_->isSelected());

        update();
      }
    }
    else if (moveConnection_) {
      pressConnection_ = nearestConnection(pressPoint_);

      if (pressConnection_) {
        deselectAll();

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

        redraw();
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

    addAction(menu, "Expand"  , SLOT(expandSlot  ()));
    addAction(menu, "Collapse", SLOT(collapseSlot()));

    menu->addSeparator();

    addAction(menu, "Place", SLOT(placeSlot()));

    menu->exec(mapToGlobal(e->pos()));
  }
}

void
Schematic::
mouseMoveEvent(QMouseEvent *e)
{
  movePoint_ = QPointF(e->x(), e->y());

  window_->setPos(renderer_.pixelToWindow(movePoint_));

  double dx = std::abs(pressPoint_.x() - movePoint_.x());
  double dy = std::abs(pressPoint_.y() - movePoint_.y());

  if (dx < 1 && dy < 1)
    return;

  if (! pressed_) {
    if      (moveGate_) {
      Gate *gate = nearestGate(movePoint_);

      if (gate != insideGate_) {
        insideGate_ = gate;

        update();
      }
    }
    else if (movePlacement_) {
      PlacementGroup *placement = nearestPlacementGroup(movePoint_);

      if (placement != insidePlacement_) {
        insidePlacement_ = placement;

        update();
      }
    }
    else if (moveConnection_) {
      Connection *connection = nearestConnection(movePoint_);

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

    redraw();
  }
  else if (pressPlacement_) {
    QPointF p1 = renderer_.pixelToWindow(QPointF(pressPoint_.x(), pressPoint_.y()));
    QPointF p2 = renderer_.pixelToWindow(QPointF(movePoint_ .x(), movePoint_ .y()));

    pressPlacement_->setRect(pressPlacement_->rect().translated(p2.x() - p1.x(), p2.y() - p1.y()));

    calcBounds();

    redraw();
  }

  pressPoint_ = movePoint_;
}

void
Schematic::
mouseReleaseEvent(QMouseEvent *e)
{
  mouseMoveEvent(e);

  pressed_ = false;
}

void
Schematic::
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

  redraw();
}

void
Schematic::
deselectAll()
{
  Gates selGates;

  selectedGates(selGates);

  for (auto &gate : selGates)
    gate->setSelected(false);

  Connections selConnections;

  selectedConnections(selConnections);

  for (auto &connection : selConnections)
    connection->setSelected(false);

  PlacementGroups selPlacementGroups;

  selectedPlacementGroups(selPlacementGroups);

  for (auto &placementGroup : selPlacementGroups)
    placementGroup->setSelected(false);
}

void
Schematic::
expandSlot()
{
  resetObjs();

  PlacementGroups expandGroups;

  PlacementGroups selPlacementGroups;

  selectedPlacementGroups(selPlacementGroups);

  if (! selPlacementGroups.empty()) {
    for (auto &placementGroup : selPlacementGroups) {
      if (placementGroup->expandName() != "")
        expandGroups.push_back(placementGroup);
    }
  }
  else {
    Gates selGates;

    selectedGates(selGates);

    if (! selGates.empty()) {
      for (auto &gate : selGates) {
        PlacementGroup *placementGroup = gate->placementGroup();

        if (placementGroup->expandName() != "")
          expandGroups.push_back(placementGroup);
      }
    }
    else {
      for (auto &placementGroupData : placementGroup_->placementGroups()) {
        PlacementGroup *placementGroup = placementGroupData.placementGroup;

        if (placementGroup->expandName() != "")
          expandGroups.push_back(placementGroup);
      }
    }
  }

  //---

  PlacementGroups newPlacementGroups;

  for (auto &placementGroup : expandGroups) {
    PlacementGroup *parentGroup = placementGroup->parent();

    bool rc = execGate(parentGroup, placementGroup->expandName());
    assert(rc);

    PlacementGroup *newPlacementGroup =
      parentGroup->replacePlacementGroup(this, placementGroup);

    newPlacementGroups.push_back(newPlacementGroup);
  }

  //---

  for (auto &placementGroup : newPlacementGroups)
    placementGroup->place();

  calcBounds();

  redraw();
}

void
Schematic::
collapseSlot()
{
  resetObjs();

  PlacementGroups selPlacementGroups;

  selectedPlacementGroups(selPlacementGroups);

  PlacementGroups collapseGroups;

  if (! selPlacementGroups.empty()) {
    for (auto &placementGroup : selPlacementGroups) {
      if (placementGroup->collapseName() != "")
        collapseGroups.push_back(placementGroup);
    }
  }
  else {
    Gates selGates;

    selectedGates(selGates);

    if (! selGates.empty()) {
      for (auto &gate : selGates) {
        PlacementGroup *placementGroup = gate->placementGroup();

        if (placementGroup->collapseName() != "")
          collapseGroups.push_back(placementGroup);
      }
    }
    else {
      for (auto &placementGroupData : placementGroup_->placementGroups()) {
        PlacementGroup *placementGroup = placementGroupData.placementGroup;

        if (placementGroup->collapseName() != "")
          collapseGroups.push_back(placementGroup);
      }
    }
  }

  //---

  PlacementGroups newPlacementGroups;

  for (auto &placementGroup : collapseGroups) {
    PlacementGroup *parentGroup = placementGroup->parent();

    bool rc = execGate(parentGroup, placementGroup->collapseName());
    assert(rc);

    PlacementGroup *newPlacementGroup =
      parentGroup->replacePlacementGroup(this, placementGroup);

    newPlacementGroups.push_back(newPlacementGroup);
  }

  //---

  for (auto &placementGroup : newPlacementGroups)
    placementGroup->place();

  calcBounds();

  redraw();
}

void
Schematic::
placeSlot()
{
  place();

  redraw();
}

void
Schematic::
resetObjs()
{
  pressGate_        = nullptr;
  pressPlacement_   = nullptr;
  insideGate_       = nullptr;
  insidePlacement_  = nullptr;
  insideConnection_ = nullptr;
}

void
Schematic::
redraw()
{
  changed_ = true;

  update();
}

void
Schematic::
selectedGates(Gates &gates) const
{
  for (auto &gate : gates_) {
    if (gate->isSelected())
      gates.push_back(gate);
  }
}

void
Schematic::
selectedConnections(Connections &connections) const
{
  for (auto &connection : connections_) {
    if (connection->isSelected())
      connections.push_back(connection);
  }
}

void
Schematic::
selectedBuses(Buses &buses) const
{
  for (const auto &bus : buses) {
    if (bus->isSelected())
      buses.push_back(bus);
  }
}

void
Schematic::
selectedPlacementGroups(PlacementGroups &placementGroups) const
{
  for (auto &placementGroupData : placementGroup_->placementGroups()) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (placementGroup->isSelected())
      placementGroups.push_back(placementGroup);
  }
}

Gate *
Schematic::
nearestGate(const QPointF &p) const
{
  for (auto &gate : gates_) {
    if (gate->inside(p))
      return gate;
  }

  return nullptr;
}

PlacementGroup *
Schematic::
nearestPlacementGroup(const QPointF &p) const
{
  return placementGroup_->nearestPlacementGroup(p);
}

Connection *
Schematic::
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
Schematic::
drawConnection(Renderer *renderer, const QPointF &p1, const QPointF &p2)
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  if (dx > dy) {
    if (dy > 1E-6) {
      double ym = (p1.y() + p2.y())/2.0;

      Schematic::drawLine(renderer, p1                 , QPointF(p1.x(), ym));
      Schematic::drawLine(renderer, QPointF(p1.x(), ym), QPointF(p2.x(), ym));
      Schematic::drawLine(renderer, QPointF(p2.x(), ym), p2                 );
    }
    else
      Schematic::drawLine(renderer, p1, p2);
  }
  else {
    if (dx > 1E-6) {
      double xm = (p1.x() + p2.x())/2.0;

      Schematic::drawLine(renderer, p1                 , QPointF(xm, p1.y()));
      Schematic::drawLine(renderer, QPointF(xm, p1.y()), QPointF(xm, p2.y()));
      Schematic::drawLine(renderer, QPointF(xm, p2.y()), p2                 );
    }
    else
      Schematic::drawLine(renderer, p1, p2);
  }
}

void
Schematic::
drawTextInRect(Renderer *renderer, const QRectF &r, const QString &text)
{
  if (r.height() < 3)
    return;

  renderer->setFontSize(r.height());

  QPointF c = r.center();

  drawTextAtPoint(renderer, c, text);
}

void
Schematic::
drawTextOnLine(Renderer *renderer, const QPointF &p1, const QPointF &p2,
               const QString &name, TextLinePos pos)
{
  if      (pos == TextLinePos::START) {
    QPointF pt(std::min(p1.x(), p2.x()) - 4, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name, Schematic::TextAlign::RIGHT);
  }
  else if (pos == TextLinePos::END) {
    QPointF pt(std::max(p1.x(), p2.x()) + 4, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name, Schematic::TextAlign::LEFT);
  }
  else {
    QPointF pt((p1.x() + p2.x())/2, (p1.y() + p2.y())/2);

    drawTextAtPoint(renderer, pt, name);
  }
}

void
Schematic::
drawTextAtPoint(Renderer *renderer, const QPointF &p, const QString &text,
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
Schematic::
drawLine(Renderer *renderer, const QPointF &p1, const QPointF &p2)
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  assert(int(dx) <= 1 || int(dy) <= 1);

  renderer->painter->drawLine(p1, p2);
}

QSize
Schematic::
sizeHint() const
{
  return QSize(800, 800);
}

//---

Waveform::
Waveform(Schematic *schem) :
 schem_(schem)
{
  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  setMouseTracking(true);

  schem_->setWaveform(this);
}

void
Waveform::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::black);

  //--

  QFontMetrics fm(font());

  int w = 0;

  for (auto &ci : connInds_) {
    const QString &name = ci.first->name();

    w = std::max(w, fm.width(name));
  }

  //--

  int dx = 1;
  int dy = fm.height();

  int dy1 = dy - 4;

  int x = w + 4;
  int y = 4;

  for (auto &iv : indValues_) {
    const Connection *connection = indConns_[iv.first];

    const QString &name = connection->name();

    painter.setPen(Qt::white);

    painter.drawText(2, y + fm.ascent(), name);

    //---

    QPainterPath path;

    int  lastT     = 0;
    bool lastValue = false;

    path.moveTo(x, y);

    for (auto &tv : iv.second) {
      int  t     = tv.first;
      bool value = tv.second;

      if (t == lastT) {
        path.lineTo(x + dx*t, y + dy1*lastValue);
        path.lineTo(x + dx*t, y + dy1*value    );
      }
      else {
        path.lineTo(x + dx*lastT, y + dy1*value);
        path.lineTo(x + dx*t    , y + dy1*value);
      }

      lastT     = t;
      lastValue = value;
    }

    painter.drawPath(path);

    y += 16;
  }
}

void
Waveform::
addValue(const Connection *connection, int t, bool b)
{
  auto p = connInds_.find(connection);

  if (p == connInds_.end()) {
    p = connInds_.insert(p, ConnInds::value_type(connection, indValues_.size()));

    indConns_[(*p).second] = connection;
  }

  indValues_[(*p).second][t] = b;
}

QSize
Waveform::
sizeHint() const
{
  return QSize(100, 800);
}

//---

Gate::
Gate(const QString &name) :
 name_(name)
{
}

Gate::
~Gate()
{
  for (auto &port : inputs_)
    delete port;

  for (auto &port : outputs_)
    delete port;
}

QSizeF
Gate::
calcSize() const
{
  return QSizeF(width(), height());
}

void
Gate::
draw(Renderer *renderer) const
{
  if (! renderer->schem->isGateVisible())
    return;

  if (renderer->schem->isShowGateText()) {
    renderer->painter->setPen(renderer->textColor);

    Schematic::drawTextInRect(renderer, prect_, name());
  }

//renderer->painter->setPen(Qt::red);
//renderer->painter->drawRect(prect_);

  if (renderer->schem->isShowPortText()) {
    renderer->painter->setPen(renderer->textColor);

    for (auto &port : inputs())
      Schematic::drawTextAtPoint(renderer, port->pixelPos(), port->name());

    for (auto &port : outputs())
      Schematic::drawTextAtPoint(renderer, port->pixelPos(), port->name());
  }
}

void
Gate::
setBrush(Renderer *renderer) const
{
  if (! renderer->selected && ! renderer->inside)
    renderer->painter->setBrush(renderer->gateFillColor);
  else
    renderer->painter->setBrush(Qt::NoBrush);
}

void
Gate::
drawRect(Renderer *renderer) const
{
  setBrush(renderer);

  renderer->painter->drawRect(prect_);
}

void
Gate::
drawAnd(Renderer *renderer) const
{
  drawAnd(renderer, px1(), py1(), px2(), py2());
}

void
Gate::
drawAnd(Renderer *renderer, double x1, double y1, double x2, double y2) const
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

  setBrush(renderer);

  renderer->painter->drawPath(path);
}

void
Gate::
drawOr(Renderer *renderer) const
{
  double x3, y3;

  drawOr(renderer, x3, y3);
}

void
Gate::
drawOr(Renderer *renderer, double &x3, double &y3) const
{
  drawOr(renderer, px1(), py1(), px2(), py2(), x3, y3);
}

void
Gate::
drawOr(Renderer *renderer, double x1, double y1, double x2, double y2,
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

  setBrush(renderer);

  renderer->painter->drawPath(path);
}

void
Gate::
drawXor(Renderer *renderer) const
{
  drawXor(renderer, px1(), py1(), px2(), py2());
}

void
Gate::
drawXor(Renderer *renderer, double x1, double y1, double x2, double y2) const
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
Gate::
drawNot(Renderer *renderer) const
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

  setBrush(renderer);

  renderer->painter->drawPath(path);
}

void
Gate::
drawNotIndicator(Renderer *renderer) const
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
Gate::
drawAdder(Renderer *renderer) const
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

  setBrush(renderer);

  renderer->painter->drawPath(path);
}

void
Gate::
placePorts(int ni, int no) const
{
  placePorts(px1(), py1(), px2(), py2(),
             px1(), py1(), px2(), py2(), ni, no);
}

void
Gate::
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

    Side iside = (orientation() == Orientation::R0 ? Side::LEFT  : Side::RIGHT);
    Side oside = (orientation() == Orientation::R0 ? Side::RIGHT : Side::LEFT );

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

    Side iside = (orientation() == Orientation::R90 ? Side::TOP    : Side::BOTTOM);
    Side oside = (orientation() == Orientation::R90 ? Side::BOTTOM : Side::TOP   );

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
Gate::
placePortOnSide(Port *port, const Side &side) const
{
  Side side1 = Side::NONE;

  if      (orientation() == Orientation::R0)
    side1 = side;
  else if (orientation() == Orientation::R90)
    side1 = Port::nextSide(side);
  else if (orientation() == Orientation::R180)
    side1 = Port::nextSide(Port::nextSide(side));
  else if (orientation() == Orientation::R270)
    side1 = Port::prevSide(side);

  port->setSide(side1);

  if      (side1 == Side::LEFT)
    port->setPixelPos(QPointF(px1(), pym()));
  else if (side1 == Side::TOP)
    port->setPixelPos(QPointF(pxm(), py1()));
  else if (side1 == Side::RIGHT)
    port->setPixelPos(QPointF(px2(), pym()));
  else if (side1 == Side::BOTTOM)
    port->setPixelPos(QPointF(pxm(), py2()));
}

void
Gate::
placePortsOnSide(Port **ports, int n, const Side &side) const
{
  Side side1 = Side::NONE;

  if      (orientation() == Orientation::R0)
    side1 = side;
  else if (orientation() == Orientation::R90)
    side1 = Port::nextSide(side);
  else if (orientation() == Orientation::R180)
    side1 = Port::nextSide(Port::nextSide(side));
  else if (orientation() == Orientation::R270)
    side1 = Port::prevSide(side);

  for (int i = 0; i < n; ++i)
    ports[i]->setSide(side1);

  if (side1 == Side::LEFT || side1 == Side::RIGHT) {
    bool flip = (side1 == Side::RIGHT);

    if (isFlipped())
      flip = ! flip;

    std::vector<double> y = calcYGaps(n, flip);

    for (int i = 0; i < n; ++i) {
      if (side1 == Side::LEFT)
        ports[i]->setPixelPos(QPointF(px1(), y[i]));
      else
        ports[i]->setPixelPos(QPointF(px2(), y[i]));
    }
  }
  else {
    bool flip = (side1 == Side::TOP);

    std::vector<double> x = calcXGaps(n, flip);

    for (int i = 0; i < n; ++i) {
      if (side1 == Side::TOP)
        ports[i]->setPixelPos(QPointF(x[i], py1()));
      else
        ports[i]->setPixelPos(QPointF(x[i], py2()));
    }
  }
}

void
Gate::
connect(const QString &name, Connection *connection)
{
  Port *port = getPortByName(name);
  assert(port);

  if (port->direction() == Direction::IN)
    connection->addOutPort(port);
  else
    connection->addInPort(port);

  port->setConnection(connection);
}

void
Gate::
addInputPort(const QString &name)
{
  Port *port = new Port(name, Direction::IN);

  port->setGate(this);

  inputs_.push_back(port);
}

void
Gate::
addOutputPort(const QString &name)
{
  Port *port = new Port(name, Direction::OUT);

  port->setGate(this);

  outputs_.push_back(port);
}

Port *
Gate::
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
Gate::
initRect(Renderer *renderer) const
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
Gate::
penColor(Renderer *renderer) const
{
  if (renderer->schem->insideGate() == this)
    return renderer->insideColor;

  return (isSelected() ? renderer->selectColor : renderer->gateStrokeColor);
}

//---

NandGate::
NandGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
NandGate::
exec()
{
  bool b = ! (inputs_[0]->getValue() & inputs_[1]->getValue());

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
NandGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

NotGate::
NotGate(const QString &name) :
 Gate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  addInputPort ("a");
  addOutputPort("c");
}

bool
NotGate::
exec()
{
  bool b = ! inputs_[0]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
NotGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

AndGate::
AndGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
AndGate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
AndGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

And3Gate::
And3Gate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "c");

  addOutputPort("d");
}

bool
And3Gate::
exec()
{
  bool b = inputs_[0]->getValue() & inputs_[1]->getValue() & inputs_[2]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
And3Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

And4Gate::
And4Gate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "c" << "d");

  addOutputPort("e");
}

bool
And4Gate::
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
And4Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

And8Gate::
And8Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(And8Gate::iname(i));

  addOutputPort("o");
}

bool
And8Gate::
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
And8Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

OrGate::
OrGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
OrGate::
exec()
{
  bool b = inputs_[0]->getValue() | inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
OrGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

Or8Gate::
Or8Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(Or8Gate::iname(i));

  addOutputPort("o");
}

bool
Or8Gate::
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
Or8Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

XorGate::
XorGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPort("c");
}

bool
XorGate::
exec()
{
  bool b = inputs_[0]->getValue() ^ inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
XorGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

MemoryGate::
MemoryGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "i" << "s");

  addOutputPort("o");
}

bool
MemoryGate::
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
MemoryGate::
draw(Renderer *renderer) const
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
  if (sside() == Side::LEFT)
    placePorts();
  else {
    placePorts(1, 1);

    placePortOnSide(inputs_[1], sside());
  }

  Gate::draw(renderer);
}

//---

Memory8Gate::
Memory8Gate(const QString &name) :
 Gate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = Memory8Gate::iname(i);
    QString oname = Memory8Gate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPort("s");
}

bool
Memory8Gate::
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
Memory8Gate::
draw(Renderer *renderer) const
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
  placePortOnSide(inputs_[8], Side::BOTTOM);

  Gate::draw(renderer);
}

//---

EnablerGate::
EnablerGate(const QString &name) :
 Gate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = EnablerGate::iname(i);
    QString oname = EnablerGate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPort("e");
}

bool
EnablerGate::
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
EnablerGate::
draw(Renderer *renderer) const
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
  placePortOnSide(inputs_[8], Side::BOTTOM);

  Gate::draw(renderer);
}

//---

RegisterGate::
RegisterGate(const QString &name) :
 Gate(name)
{
  w_ = 0.5;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i) {
    QString iname = RegisterGate::iname(i);
    QString oname = RegisterGate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPorts(QStringList() << "s" << "e");
}

bool
RegisterGate::
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
RegisterGate::
draw(Renderer *renderer) const
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
  placePortsOnSide(const_cast<Port **>(&inputs_[8]), 2, Side::BOTTOM);

  Gate::draw(renderer);
}

//---

Decoder4Gate::
Decoder4Gate(const QString &name) :
 Gate(name)
{
  w_ = 2.00/4.0;
  h_ = 1.00;

  addInputPorts(QStringList() << "a" << "b");

  for (int i = 0; i < 4; ++i) {
    QString oname = Decoder4Gate::oname(i);

    addOutputPort(oname);
  }
}

bool
Decoder4Gate::
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
Decoder4Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

Decoder8Gate::
Decoder8Gate(const QString &name) :
 Gate(name)
{
  w_ = 3.00/8.0;
  h_ = 1.00;

  addInputPorts(QStringList() << "a" << "b" << "c");

  for (int i = 0; i < 8; ++i) {
    QString oname = Decoder8Gate::oname(i);

    addOutputPort(oname);
  }
}

bool
Decoder8Gate::
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
Decoder8Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

Decoder16Gate::
Decoder16Gate(const QString &name) :
 Gate(name)
{
  w_ = 4.00/16.0;
  h_ = 1.0;

  for (int i = 0; i < 4; ++i)
    addInputPort(Decoder16Gate::iname(i));

  for (int i = 0; i < 16; ++i)
    addOutputPort(Decoder16Gate::oname(i));
}

bool
Decoder16Gate::
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
Decoder16Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

Decoder256Gate::
Decoder256Gate(const QString &name) :
 Gate(name)
{
  w_ = 16.00/256.0;
  h_ = 1.0;

  for (int i = 0; i < 8; ++i)
    addInputPort(Decoder256Gate::iname(i));

  for (int i = 0; i < 256; ++i)
    addOutputPort(Decoder256Gate::oname(i));
}

bool
Decoder256Gate::
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
Decoder256Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

AdderGate::
AdderGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b" << "carry_in");

  addOutputPorts(QStringList() << "sum" << "carry_out");
}

bool
AdderGate::
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
AdderGate::
draw(Renderer *renderer) const
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
  placePortOnSide(inputs_[2], Side::BOTTOM);

  // place ports carry_in on top
  placePortOnSide(outputs_[1], Side::TOP);

  Gate::draw(renderer);
}

//---

Adder8Gate::
Adder8Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(Adder8Gate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(Adder8Gate::bname(i));

  addInputPort("carry_in");

  for (int i = 0; i < 8; ++i)
    addOutputPort(Adder8Gate::cname(i));

  addOutputPort("carry_out");
}

bool
Adder8Gate::
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
Adder8Gate::
draw(Renderer *renderer) const
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
  placePortOnSide(inputs_[16], Side::BOTTOM);

  // place ports carry_in on top
  placePortOnSide(outputs_[8], Side::TOP);

  Gate::draw(renderer);
}

//---

ComparatorGate::
ComparatorGate(const QString &name) :
 Gate(name)
{
  addInputPorts(QStringList() << "a" << "b");

  addOutputPorts(QStringList() << "c" << "a_larger" << "equal");
}

bool
ComparatorGate::
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
ComparatorGate::
draw(Renderer *renderer) const
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
  placePortsOnSide(const_cast<Port **>(&outputs_[1]), 2, Side::TOP);

  Gate::draw(renderer);
}

//---

Comparator8Gate::
Comparator8Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(Adder8Gate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(Adder8Gate::bname(i));

  for (int i = 0; i < 8; ++i)
    addOutputPort(Adder8Gate::cname(i));

  addOutputPorts(QStringList() << "a_larger" << "equal");
}

bool
Comparator8Gate::
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
      outputs_[i]->setValue(c[i]);

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
Comparator8Gate::
draw(Renderer *renderer) const
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
  placePortsOnSide(const_cast<Port **>(&outputs_[8]), 2, Side::TOP);

  Gate::draw(renderer);
}

//---

Bus0Gate::
Bus0Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(Bus0Gate::iname(i));

  addOutputPort("zero");
}

bool
Bus0Gate::
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
Bus0Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

Bus1Gate::
Bus1Gate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(Bus1Gate::iname(i));

  addInputPort("bus1");

  for (int i = 0; i < 8; ++i)
    addOutputPort(Bus1Gate::oname(i));
}

bool
Bus1Gate::
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
Bus1Gate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

AluGate::
AluGate(const QString &name) :
 Gate(name)
{
  for (int i = 0; i < 8; ++i)
    addInputPort(AluGate::aname(i));

  for (int i = 0; i < 8; ++i)
    addInputPort(AluGate::bname(i));

  addInputPort("carry_in");

  for (int i = 0; i < 3; ++i)
    addInputPort(AluGate::opname(i));

  for (int i = 0; i < 8; ++i)
    addOutputPort(AluGate::cname(i));

  addOutputPort("carry_out");
  addOutputPort("a_larger");
  addOutputPort("equal");
  addOutputPort("zero");
}

bool
AluGate::
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
AluGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

LShiftGate::
LShiftGate(const QString &name) :
 Gate(name)
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
LShiftGate::
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
LShiftGate::
draw(Renderer *renderer) const
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

  setBrush(renderer);

  renderer->painter->drawPath(path);

  //---

  // place ports and draw connections
  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180)
    placePorts(px1(), y3, px1(), py2(), px2(), py1(), px2(), y4, 9, 9);
  else
    placePorts(px1(), py1(), x4, py1(), x3, py2(), px2(), py2(), 9, 9);

  // place ports s and e on bottom
  placePortsOnSide(const_cast<Port **>(&inputs_[9]), 2, Side::BOTTOM);

  Gate::draw(renderer);
}

//---

RShiftGate::
RShiftGate(const QString &name) :
 Gate(name)
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
RShiftGate::
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
RShiftGate::
draw(Renderer *renderer) const
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

  setBrush(renderer);

  renderer->painter->drawPath(path);

  //---

  // place ports and draw connections
  if      (orientation() == Orientation::R0 || orientation() == Orientation::R180)
    placePorts(px1(), py1(), px1(), y4, px2(), y3, px2(), py2(), 9, 9);
  else
    placePorts(x3, py1(), px2(), py1(), px2(), py2(), x4, py2(), 9, 9);

  // place ports s and e on bottom
  placePortsOnSide(const_cast<Port **>(&inputs_[9]), 2, Side::BOTTOM);

  Gate::draw(renderer);
}

//---

InverterGate::
InverterGate(const QString &name) :
 Gate(name)
{
  w_ = 0.5;
  h_ = 0.5;

  for (int i = 0; i < 8; ++i) {
    addInputPort (iname(i));
    addOutputPort(oname(i));
  }
}

bool
InverterGate::
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
InverterGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

AnderGate::
AnderGate(const QString &name) :
 Gate(name)
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
AnderGate::
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
AnderGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

OrerGate::
OrerGate(const QString &name) :
 Gate(name)
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
OrerGate::
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
OrerGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

XorerGate::
XorerGate(const QString &name) :
 Gate(name)
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
XorerGate::
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
XorerGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

ClkGate::
ClkGate(const QString &name) :
 Gate(name)
{
  addOutputPort("clk");
}

bool
ClkGate::
exec()
{
  if (delay1_ > 0) {
    --delay1_;

    return false;
  }

  if (cycle1_ > 0) {
    --cycle1_;

    return false;
  }

  state_ = ! state_;

  outputs_[0]->setValue(state_);

  cycle1_ = cycle_;

  return true;
}

void
ClkGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

ClkESGate::
ClkESGate(const QString &name) :
 Gate(name)
{
  addOutputPort("clk"  );
  addOutputPort("clk_e");
  addOutputPort("clk_s");

  delay1t_ = delay1_;
  cycle1t_ = cycle1_;

  delay2t_ = delay2_;
  cycle2t_ = cycle2_;
}

bool
ClkESGate::
exec()
{
  bool set1 = true;
  bool set2 = true;

  if (delay1t_ > 0) { --delay1t_; set1 = false; }
  if (delay2t_ > 0) { --delay2t_; set2 = false; }

  if (set1 && cycle1t_ > 0) { --cycle1t_; set1 = false; }
  if (set2 && cycle2t_ > 0) { --cycle2t_; set2 = false; }

  if (set1) {
    state1_  = ! state1_;
    cycle1t_ = cycle1_;
  }

  if (set2) {
    state2_  = ! state2_;
    cycle2t_ = cycle2_;
  }

  outputs_[0]->setValue(state1_); // clk

  bool state3 = state1_ & state2_;
  bool state4 = state1_ | state2_;

  outputs_[1]->setValue(state3); // clk_e
  outputs_[2]->setValue(state4); // clk_s

  return true;
}

void
ClkESGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//---

StepperGate::
StepperGate(const QString &name) :
 Gate(name)
{
  addInputPort("clk"  );
  addInputPort("reset");

  addOutputPort("1");
  addOutputPort("2");
  addOutputPort("3");
  addOutputPort("4");
  addOutputPort("5");
  addOutputPort("6");
  addOutputPort("7");
}

bool
StepperGate::
exec()
{
  bool c = inputs_[0]->getValue();
  bool r = inputs_[1]->getValue();

  if (r) {
    state_ = 0;
  }
  else {
    if (c != b_) {
      if (c) {
        if (state_ < 7)
          ++state_;
      }

      b_ = c;
    }
  }

  bool changed = false;

  for (int i = 0; i < 7; ++i) {
    bool b1 = (state_ == i + 1);

    if (outputs_[i]->getValue() != b1) {
      outputs_[i]->setValue(b1);

      changed = true;
    }
  }

  return changed;
}

void
StepperGate::
draw(Renderer *renderer) const
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

  Gate::draw(renderer);
}

//------

void
Port::
setValue(bool b, bool propagate)
{
  value_ = b;

  if (propagate && connection_)
    connection_->setValue(b);
}

QPointF
Port::
offsetPixelPos() const
{
  static int dl = 32;

  Side side = calcSide();

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

Side
Port::
calcSide() const
{
  Side side = side_;

  if (side == Side::NONE)
    side = (direction_ == Direction::IN ? Side::LEFT : Side::RIGHT);

  return side;
}

//------

Connection::
Connection(const QString &name) :
 name_(name)
{
}

void
Connection::
setValue(bool b)
{
  value_ = b;

  // propagate to output
  for (auto &oport : outPorts())
    oport->setValue(getValue(), false);

  if (isTraced())
    schem_->addTValue(this, b);
}

bool
Connection::
isLR() const
{
  Side side { Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == Side::LEFT || side == Side::RIGHT);
}

bool
Connection::
isLeft() const
{
  Side side { Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == Side::LEFT);
}

bool
Connection::
isTop() const
{
  Side side { Side::NONE };

  if      (! inPorts_ .empty())
    side = inPorts_ [0]->side();
  else if (! outPorts_.empty())
    side = outPorts_[0]->side();
  else
    return true;

  return (side == Side::TOP);
}

void
Connection::
merge(Connection *connection)
{
  for (auto &port : connection->inPorts_)
    addInPort(port);

  for (auto &port : connection->outPorts_)
    addOutPort(port);

  connection->inPorts_ .clear();
  connection->outPorts_.clear();
}

void
Connection::
removePort(Port *port)
{
  int i = 0;
  int n = inPorts_.size();

  for ( ; i < n; ++i)
    if (inPorts_[i] == port)
      break;

  if (i < n) {
    ++i;

    for ( ; i < n; ++i)
      inPorts_[i - 1] = inPorts_[i];

    inPorts_.pop_back();

    return;
  }

  i = 0;
  n = outPorts_.size();

  for ( ; i < n; ++i)
    if (outPorts_[i] == port)
      break;

  if (i < n) {
    ++i;

    for ( ; i < n; ++i)
      outPorts_[i - 1] = outPorts_[i];

    outPorts_.pop_back();

    return;
  }

  assert(false);
}

bool
Connection::
inside(const QPointF &p) const
{
  //return prect_.contains(p);

  for (const auto &line : lines_) {
    if (line.contains(p))
      return true;
  }

  return false;
}

void
Connection::
draw(Renderer *renderer) const
{
  if (! renderer->schem->isConnectionVisible())
    return;

  int ni = inPorts_ .size();
  int no = outPorts_.size();

  if (ni == 0 && no == 0)
    return;

  //---

  SidePoints points;

  for (const auto &port : inPorts_) {
    QPointF     p    = port->pixelPos();
    const Side &side = port->calcSide();

    points.push_back(SidePoint(QPoint(p.x(), p.y()), side, Direction::OUT));
  }

  for (const auto &port : outPorts_) {
    QPointF     p    = port->pixelPos();
    const Side &side = port->calcSide();

    points.push_back(SidePoint(QPoint(p.x(), p.y()), side, Direction::IN));
  }

  lines_.clear();

  if (ni + no == 1)
    calcSinglePointLines(renderer, points, lines_);
  else if ((ni > 1 && no == 0) || (no > 1 && ni == 0))
    calcSingleDirectionLines(renderer, points, lines_);
  else
    calcLines(renderer, points, lines_);

  //---

#if 0
  double ym = (renderer->rect.top() + renderer->rect.bottom())/2.0;

  QPointF p1(renderer->rect.left (), ym);
  QPointF p2(renderer->rect.right(), ym);

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
#endif

  //---

  // find line for text
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

  //---

  // draw lines
  renderer->painter->setPen(penColor(renderer));

  for (const auto &line : lines_)
    drawLine(renderer, line.start, line.end, /*showText*/ line.ind == ind);
}

void
Connection::
calcSinglePointLines(Renderer *, const SidePoints &points, Lines &lines) const
{
  static const int DS = 32;

  assert(points.size() == 1);

  // single point just add unconnected line
  QPoint p1    = points.begin()->p;
  Side   side1 = points.begin()->side;

  QPoint p2;

  if      (side1 == Side::LEFT  ) { p2 = QPoint(p1.x() - DS, p1.y()); }
  else if (side1 == Side::RIGHT ) { p2 = QPoint(p1.x() + DS, p1.y()); }
  else if (side1 == Side::TOP   ) { p2 = QPoint(p1.x(), p1.y() - DS); }
  else if (side1 == Side::BOTTOM) { p2 = QPoint(p1.x(), p1.y() + DS); }

  addLine(lines, p1, p2);

  return;
}

void
Connection::
calcSingleDirectionLines(Renderer *, const SidePoints &points, Lines &lines) const
{
  static const int DS = 32;

  int np = points.size();

  // calc average port connection point
  double xo = 0.0;
  double yo = 0.0;

  int xmin = 0, ymin = 0, xmax = 0, ymax = 0;

  Side side       = Side::NONE;
  bool singleSide = true;

  for (const auto &dp : points) {
    if      (side == Side::NONE) {
      xmin = dp.p.x(); xmax = xmin;
      ymin = dp.p.y(); ymax = ymin;

      side = dp.side;
    }
    else if (side != dp.side)
      singleSide = false;

    QPointF p;

    if      (dp.side == Side::LEFT) {
      p = QPointF(dp.p.x() - 2*DS, dp.p.y());

      xmin = std::min(xmin, dp.p.x() - 2*DS);
    }
    else if (dp.side == Side::RIGHT) {
      p = QPointF(dp.p.x() + 2*DS, dp.p.y());

      xmax = std::max(xmax, dp.p.x() + 2*DS);
    }
    else if (dp.side == Side::TOP) {
      p = QPointF(dp.p.x(), dp.p.y() - 2*DS);

      ymin = std::min(ymin, dp.p.y() - 2*DS);
    }
    else if (dp.side == Side::BOTTOM) {
      p = QPointF(dp.p.x(), dp.p.y() + 2*DS);

      ymax = std::max(ymax, dp.p.y() + 2*DS);
    }
    else
      assert(false);

    xo += p.x();
    yo += p.y();
  }

  xo /= np;
  yo /= np;

  double xo1 = xo;
  double yo1 = yo;

  if (singleSide) {
    if      (side == Side::LEFT  ) { xo = xmin; xo1 = xmin - DS; }
    else if (side == Side::RIGHT ) { xo = xmax; xo1 = xmax + DS; }
    else if (side == Side::TOP   ) { yo = ymin; yo1 = ymin - DS; }
    else if (side == Side::BOTTOM) { yo = ymax; yo1 = ymax - DS; }
    else assert(false);
  }

  QPointF po (xo , yo );
  QPointF po1(xo1, yo1);

  // draw lines
  for (const auto &dp : points) {
    QPointF p;

    if      (dp.side == Side::LEFT)
      p = QPointF(dp.p.x() - DS, dp.p.y());
    else if (dp.side == Side::RIGHT)
      p = QPointF(dp.p.x() + DS, dp.p.y());
    else if (dp.side == Side::TOP)
      p = QPointF(dp.p.x(), dp.p.y() - DS);
    else if (dp.side == Side::BOTTOM)
      p = QPointF(dp.p.x(), dp.p.y() + DS);
    else
      assert(false);

    addLine(lines, dp.p, p);

    connectPoints(p, po);
  }

  addLine(lines, po1, po);
}

void
Connection::
calcLines(Renderer *renderer, const SidePoints &points, Lines &lines) const
{
  static const int DS = 32;

  assert(points.size() > 1);

  //---

  struct GridNode {
    int  ncon { -1 };
    Side side { Side::NONE };

    GridNode(int ncon=-1, const Side &side=Side::NONE) :
     ncon(ncon), side(side) {
    }
  };

  class GridData {
   public:
    using IVals       = std::map<int,int>;
    using XConnected  = std::map<int,GridNode>;
    using XYConnected = std::map<int,XConnected>;

   public:
    GridData() { }

    void addPoint(const QPoint &p, int ncon=-1, const Side &side=Side::NONE) {
      auto px = xvals_.find(p.x());
      auto py = yvals_.find(p.y());

      if (px == xvals_.end())
        xvals_[p.x()] = 0;

      if (py == yvals_.end())
        yvals_[p.y()] = 0;

      auto ci = connected_.find(p.y());

      if (ci == connected_.end() || (ci->second.find(p.x()) == ci->second.end()))
        connected_[p.y()][p.x()] = GridNode(ncon, side);
    }

    void addExtraPoints(int ds) {
      int x1 = this->x1();
      int x2 = this->x2();

      int y1 = this->y1();
      int y2 = this->y2();

      //int w = std::max(x2 - x1, ds);
      //int h = std::max(y2 - y1, ds);

      int xm = (x1 + x2)/2.0;
      int ym = (y1 + y2)/2.0;

      std::set<int> xv, yv;

      for (int x = xm; x >= x1 - ds; x -= ds) xv.insert(x);
      for (int x = xm; x <= x2 + ds; x += ds) xv.insert(x);

      for (int y = ym; y >= y1 - ds; y -= ds) yv.insert(y);
      for (int y = ym; y <= y2 + ds; y += ds) yv.insert(y);

      for (const auto &y : yv)
        for (const auto &x : xv)
          addPoint(QPoint(x, y));

      //---

      for (const auto &yc : connected_) {
        for (const auto &xc : yc.second) {
          if      (xc.second.side == Side::LEFT)
            addPoint(QPoint(xc.first - ds, yc.first));
          else if (xc.second.side == Side::RIGHT)
            addPoint(QPoint(xc.first + ds, yc.first));
          else if (xc.second.side == Side::TOP)
            addPoint(QPoint(xc.first, yc.first - ds));
          else if (xc.second.side == Side::BOTTOM)
            addPoint(QPoint(xc.first, yc.first + ds));
        }
      }
    }

    void initConnected() {
      for (const auto &py : yvals_) {
        auto pcy = connected_.find(py.first);

        for (const auto &px : xvals_) {
          bool found = (pcy != connected_.end());

          if (found) {
            auto pcxy = pcy->second.find(px.first);

            found = (pcxy != pcy->second.end());
          }

          if (! found)
            connected_[py.first][px.first] = GridNode();
        }
      }
    }

    int x1() const { return xvals_.begin ()->first; }
    int x2() const { return xvals_.rbegin()->first; }
    int y1() const { return yvals_.begin ()->first; }
    int y2() const { return yvals_.rbegin()->first; }

    int prevX(int x, bool &ok) {
      auto px = xvals_.find(x);
      assert(px != xvals_.end());

      --px;

      ok = (px != xvals_.end());

      if (! ok)
        return x;

      return px->first;
    }

    int nextX(int x, bool &ok) {
      auto px = xvals_.find(x);
      assert(px != xvals_.end());

      ++px;

      ok = (px != xvals_.end());

      if (! ok)
        return x;

      return px->first;
    }

    int prevY(int y, bool &ok) {
      auto py = yvals_.find(y);
      assert(py != yvals_.end());

      --py;

      ok = (py != yvals_.end());

      if (! ok)
        return y;

      return py->first;
    }

    int nextY(int y, bool &ok) {
      auto py = yvals_.find(y);
      assert(py != yvals_.end());

      ++py;

      ok = (py != yvals_.end());

      if (! ok)
        return y;

      return py->first;
    }

    QPoint nextPoint(const QPoint &p, const Side &side, int &ncon, bool &ok) {
      ok = false;

      QPoint p1 = p;

      if      (side == Side::LEFT) {
        p1 = QPoint(prevX(p.x(), ok), p.y());
      }
      else if (side == Side::RIGHT) {
        p1 = QPoint(nextX(p.x(), ok), p.y());
      }
      else if (side == Side::TOP) {
        p1 = QPoint(p.x(), prevY(p.y(), ok));
      }
      else if (side == Side::BOTTOM) {
        p1 = QPoint(p.x(), nextY(p.y(), ok));
      }

      //---

      auto pcy = connected_.find(p1.y());
      assert(pcy != connected_.end());

      auto pcxy = pcy->second.find(p1.x());
      assert(pcxy != pcy->second.end());

      ncon = pcxy->second.ncon;

      return p1;
    }

    void setConnected(const QPoint &p, int ncon) {
      auto pcy = connected_.find(p.y());
      assert(pcy != connected_.end());

      auto pcxy = pcy->second.find(p.x());
      assert(pcxy != pcy->second.end());

      if (ncon >= 0)
        assert(pcxy->second.ncon == -1);

      pcxy->second.ncon = ncon;
    }

    void addLine(const QPoint &p1, const QPoint &p2) {
      if (cmp(p1, p2) < 0)
        lines_[p1].insert(p2);
      else
        lines_[p2].insert(p1);
    }

    bool isLine(const QPoint &p1, const QPoint &p2) const {
      assert(cmp(p1, p2) < 0);

      auto i1 = lines_.find(p1);
      if (i1 == lines_.end()) return false;

      auto i2 = i1->second.find(p2);
      if (i2 == i1->second.end()) return false;

      return true;
    }

    void print(std::ostream &os) {
      bool yfirst = true;

      int y1 = 0, y2 = 0;

      for (const auto &yc : connected_) {
        y1 = y2;
        y2 = yc.first;

        //---

        bool xfirst = true;

        if (! yfirst) {
          for (const auto &xc : yc.second) {
            int x = xc.first;

            //---

            if (! xfirst)
              os << "  ";

            if (isLine(QPoint(x, y1), QPoint(x, y2)))
              os << (xc.second.ncon >= 0 ? "#" : "#");
            else
              os << (xc.second.ncon >= 0 ? "|" : "|");

            xfirst = false;
          }

          os << "\n";
        }

        //---

        xfirst = true;

        int x1 = 0, x2 = 0;

        for (const auto &xc : yc.second) {
          x1 = x2;
          x2 = xc.first;

          //---

          if (! xfirst) {
            if (isLine(QPoint(x1, y2), QPoint(x2, y2)))
              os << "==";
            else
              os << "--";
          }

          if (xc.second.side != Side::NONE) {
            switch (xc.second.side) {
              case Side::LEFT  : os << "L"; break;
              case Side::RIGHT : os << "R"; break;
              case Side::TOP   : os << "T"; break;
              case Side::BOTTOM: os << "B"; break;
              default: assert(false);
            }
          }
          else {
            if (xc.second.ncon >= 0)
              os << xc.second.ncon;
            else
              os << ".";
          }

          xfirst = false;
        }

        //---

        yfirst = false;

        os << "\n";
      }
    }

    void draw(Renderer *renderer) const {
      renderer->painter->setPen(QColor(64,64,128,200));

      int x1 = this->x1();
      int x2 = this->x2();

      int y1 = this->y1();
      int y2 = this->y2();

      for (const auto &x : xvals_) {
        Schematic::drawLine(renderer, QPointF(x.first, y1), QPoint(x.first, y2));
      }

      for (const auto &y : yvals_) {
        Schematic::drawLine(renderer, QPointF(x1, y.first), QPoint(x2, y.first));
      }
    }

    static int cmp(const QPoint &p1, const QPoint &p2) {
      if (p1.x() <  p2.x()) return -1;
      if (p1.x() != p2.x()) return  1;
      if (p1.y() <  p2.y()) return -1;
      if (p1.y() != p2.y()) return  1;
      return 0;
    }

    struct PointCmp {
      bool operator()(const QPoint &p1, const QPoint &p2) const {
        return cmp(p1, p2) < 0;
      }
    };

   private:
    using PointSet      = std::set<QPoint,PointCmp>;
    using PointPointSet = std::map<QPoint,PointSet,PointCmp>;

    IVals         xvals_;
    IVals         yvals_;
    XYConnected   connected_;
    PointPointSet lines_;
  };

  GridData grid;

  //---

  bool debug = renderer->schem->isDebugConnect();

  //---

  int ncon = 0;

  for (const auto &dp : points) {
    grid.addPoint(dp.p, ncon, dp.side);

    ++ncon;
  }

  //---

  LinesData linesData;

#if 0
  int x1 = grid.x1();
  int x2 = grid.x2();

  int y1 = grid.y1();
  int y2 = grid.y2();

  int w = std::max(x2 - x1, DS);
  int h = std::max(y2 - y1, DS);

  for (const auto &dp : points) {
    if      (dp.side == Side::LEFT)
      grid.addPoint(dp.p - QPoint(w/2.0, 0));
    else if (dp.side == Side::RIGHT)
      grid.addPoint(dp.p + QPoint(w/2.0, 0));
    else if (dp.side == Side::TOP)
      grid.addPoint(dp.p - QPoint(0, h/2.0));
    else if (dp.side == Side::BOTTOM)
      grid.addPoint(dp.p + QPoint(0, h/2.0));
  }
#endif

  //---

  grid.addExtraPoints(DS);

  grid.initConnected();

  //---

  SidePoints points1 = points;

  ncon = 0;

  for (auto &dp : points1) {
    int  ncon1 = -1;
    bool ok    = true;

    QPoint p = grid.nextPoint(dp.p, dp.side, ncon1, ok);
    assert(ok);

    linesData.addLine(dp.p, p);

    if (ncon1 == -1)
      grid.setConnected(p, ncon);

    if (debug)
      grid.addLine(dp.p, p);

    dp.p = p;

    ++ncon;
  }

  //---

  struct TargetPoint {
    QPoint p;
    int    ncon { -1 };

    TargetPoint(const QPoint &p, int ncon) :
     p(p), ncon(ncon) {
    }
  };

  using TargetPoints = std::vector<TargetPoint>;

  TargetPoints targetPoints;

  ncon = 0;

  for (auto &dp : points1) {
    targetPoints.push_back(TargetPoint(dp.p, ncon));

    ++ncon;
  }

  //---

  auto addLineData = [&](const QPoint &p1, const QPoint &p2, int ncon) {
    linesData.addLine(p1, p2);

    if (ncon >= 0)
      targetPoints.push_back(TargetPoint(p2, ncon));

    if (ncon >= 0)
      grid.setConnected(p2, ncon);

    if (debug)
      grid.addLine(p1, p2);

    if (debug)
      grid.print(std::cerr);
  };

  //--

  // find nearest target point
  struct MinData {
    double d     { 0.0 };
    QPoint p;
    bool   found { false };
  };

  using ConMinData = std::map<int,MinData>;

  ncon = 0;

  for (const auto &dp : points1) {
    // get nearest point for each other connection
    ConMinData conMinData;

    bool minFound = false;

    for (const auto &dp1 : targetPoints) {
      if (dp1.ncon == ncon) continue;

      int ncon1 = (ncon > 0 ? 0 : dp1.ncon);

      MinData &minData = conMinData[ncon1];

      double d = std::hypot(dp1.p.x() - dp.p.x(), dp1.p.y() - dp.p.y());

      if (! minData.found || d < minData.d) {
        minData.d     = d;
        minData.p     = dp1.p;
        minData.found = true;

        minFound = true;
      }
    }

    if (! minFound)
      break;

    //---

    for (const auto &pc : conMinData) {
      const MinData &minData = pc.second;

      //---

      // get nearest point from minimum back to original connect
      MinData minData1;

      for (const auto &dp1 : targetPoints) {
        if (dp1.ncon != ncon) continue;

        double d = std::hypot(dp1.p.x() - minData.p.x(), dp1.p.y() - minData.p.y());

        if (! minData1.found || d < minData1.d) {
          minData1.d     = d;
          minData1.p     = dp1.p;
          minData1.found = true;
        }
      }

      assert(minData1.found);

      //---

      if (debug)
        grid.print(std::cerr);

      //---

      int  ncon1 = -1;
      bool ok    = true;

      QPoint p1    = minData1.p;
      Side   side1 = dp.side;
      QPoint p2    = p1;
      Side   side2 = side1;

      //---

      while (true) {
        p1    = p2;
        side1 = side2;

        int dx = std::abs(minData.p.x() - p1.x());
        int dy = std::abs(minData.p.y() - p1.y());

        if (dx == 0 && dy == 0)
          break;

        if (dx > dy) {
          if (minData.p.x() > p1.x())
            side2 = Side::RIGHT;
          else
            side2 = Side::LEFT;

          p2 = grid.nextPoint(p1, side2, ncon1, ok);

          if      (! ok) {
            side2 = otherSide(side2);

            p2 = grid.nextPoint(p1, side2, ncon1, ok);
          }
          else if (ncon1 == ncon) {
            // try perp direction
            if (minData.p.y() > p1.y())
              side2 = Side::BOTTOM;
            else
              side2 = Side::TOP;

            p2 = grid.nextPoint(p1, side2, ncon1, ok);

            if (! ok || ncon1 == ncon) {
              side2 = otherSide(side2);

              p2 = grid.nextPoint(p1, side2, ncon1, ok);

              if (ok && ncon1 == ncon)
                ok = false;
            }
          }
          else if (ncon1 == -1) {
            int ncon2;

            QPoint p3 = grid.nextPoint(p2, side2, ncon2, ok);

            while (ok && ncon2 == -1 &&
                   ((side2 == Side::LEFT  && minData.p.x() <= p3.x()) ||
                    (side2 == Side::RIGHT && minData.p.x() >= p3.x()))) {
              ncon1 = ncon2;

              addLineData(p1, p2, ncon);

              //---

              p1 = p2;
              p2 = p3;
              p3 = grid.nextPoint(p2, side2, ncon2, ok);
            }
          }
        }
        else {
          if (minData.p.y() > p1.y())
            side2 = Side::BOTTOM;
          else
            side2 = Side::TOP;

          p2 = grid.nextPoint(p1, side2, ncon1, ok);

          if      (! ok) {
            side2 = otherSide(side2);

            p2 = grid.nextPoint(p1, side2, ncon1, ok);
          }
          else if (ncon1 == ncon) {
            // try perp direction
            if (minData.p.x() > p1.x())
              side2 = Side::RIGHT;
            else
              side2 = Side::LEFT;

            p2 = grid.nextPoint(p1, side2, ncon1, ok);

            if (! ok || ncon1 == ncon) {
              side2 = otherSide(side2);

              p2 = grid.nextPoint(p1, side2, ncon1, ok);

              if (ok && ncon1 == ncon)
                ok = false;
            }
          }
          else if (ncon1 == -1) {
            int ncon2;

            QPoint p3 = grid.nextPoint(p2, side2, ncon2, ok);

            while (ok && ncon2 == -1 &&
                   ((side2 == Side::BOTTOM && minData.p.y() >= p3.y()) ||
                    (side2 == Side::TOP    && minData.p.y() <= p3.y()))) {
              ncon1 = ncon2;

              addLineData(p1, p2, ncon);

              //---

              p1 = p2;
              p2 = p3;
              p3 = grid.nextPoint(p2, side2, ncon2, ok);
            }
          }
        }

        if (! ok)
          break;

        addLineData(p1, p2, (ncon1 == -1 ? ncon : -1));
      }
    }

    ++ncon;
  }

  if (debug)
    grid.draw(renderer);

  lines = linesData.lines();
}

void
Connection::
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
Connection::
connectPoints(const QPointF &p1, const QPointF &p2) const
{
  connectPoints(lines_, p1, p2);
}

void
Connection::
connectPoints(Lines &lines, const QPointF &p1, const QPointF &p2) const
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  if (dx > dy) {
    if (dy > 1) {
      double ym = (p1.y() + p2.y())/2.0;

      QPointF p3(p1.x(), ym);
      QPointF p4(p2.x(), ym);

      addLine(lines, p1, p3);
      addLine(lines, p3, p4);
      addLine(lines, p4, p2);
    }
    else
      addLine(p1, p2);
  }
  else {
    if (dx > 1) {
      double xm = (p1.x() + p2.x())/2.0;

      QPointF p3(xm, p1.y());
      QPointF p4(xm, p2.y());

      addLine(lines, p1, p3);
      addLine(lines, p3, p4);
      addLine(lines, p4, p2);
    }
    else
      addLine(lines, p1, p2);
  }
}

void
Connection::
addLine(const QPointF &p1, const QPointF &p2) const
{
  addLine(lines_, p1, p2);
}

void
Connection::
addLine(Lines &lines, const QPointF &p1, const QPointF &p2) const
{
  double dx = std::abs(p2.x() - p1.x());
  double dy = std::abs(p2.y() - p1.y());

  assert(int(dx) <= 1 || int(dy) <= 1);

  int ind = lines.size();

  if (p1.x() > p2.x() || (p1.x() == p2.x() && p1.y() > p2.y()))
    lines.push_back(Line(ind, p2, p1));
  else
    lines.push_back(Line(ind, p1, p2));
}

void
Connection::
drawLine(Renderer *renderer, const QPointF &p1, const QPointF &p2, bool showText) const
{
  renderer->painter->setPen(penColor(renderer));

  Schematic::drawLine(renderer, p1, p2);

  if (showText && renderer->schem->isShowConnectionText()) {
    renderer->painter->setPen(renderer->textColor);

    if      (isInput() || isOutput()) {
      if (isLR()) {
        if (isLeft())
          Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::START);
        else
          Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::END);
      }
      else
        Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::MIDDLE);
    }
    else
      Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::MIDDLE);
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
Connection::
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
Connection::
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
Connection::
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
Connection::
penColor(Renderer *renderer) const
{
  if (renderer->schem->insideConnection() == this)
    return renderer->insideColor;

  if (getValue())
    return Qt::green;

  return (isSelected() ? renderer->selectColor : renderer->connectionColor);
}

//------

Bus::
Bus(const QString &name, int n) :
 name_(name), n_(n)
{
  connections_.resize(n);
}

void
Bus::
addConnection(Connection *connection, int i)
{
  assert(connection && i >= 0 && i < n_);

  connections_[i] = connection;

  connection->setBus(this);
}

int
Bus::
connectionIndex(Connection *connection)
{
  for (int i = 0; i < n_; ++i)
    if (connections_[i] == connection)
      return i;

  assert(false);

  return -1;
}

void
Bus::
draw(Renderer *renderer)
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

  if      (position() == Bus::Position::START) {
    xic = xis + offset()*(xie - xis);
    yic = yis + offset()*(yie - yis);

    xoc = xos + offset()*(xoe - xis);
    yoc = yos + offset()*(yoe - yis);
  }
  else if (position() == Bus::Position::END) {
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
      Side side = connections_[0]->outPorts()[0]->side();

      bool lr = connections_[0]->isLR();

      QPointF p1, p2;

      if (lr) {
        double dw = mapWidth(renderer->placementRect.width()/2.0);

        if (side == Side::LEFT) {
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

        if (side == Side::TOP) {
          p1 = QPointF(xic, yi1 - dh);
          p2 = QPointF(xic, yi1     );
        }
        else {
          p1 = QPointF(xic, yi1 + dh);
          p2 = QPointF(xic, yi1     );
        }
      }

      Schematic::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        if (lr) {
          if (side == Side::LEFT)
            Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::START);
          else
            Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::END);
        }
        else
          Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::MIDDLE);
      }

      return;
    }
    else if (no > 0 && ni == 0) {
      Side side = connections_[0]->inPorts()[0]->side();

      bool lr = connections_[0]->isLR();

      QPointF p1, p2;

      if (lr) {
        double dw = mapWidth(renderer->placementRect.width()/2.0);

        if (side == Side::LEFT) {
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

        if (side == Side::TOP) {
          p1 = QPointF(xoc, yo1 - dh);
          p2 = QPointF(xoc, yo1     );
        }
        else {
          p1 = QPointF(xoc, yo1 + dh);
          p2 = QPointF(xoc, yo1     );
        }
      }

      Schematic::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        if (lr) {
          if (side == Side::LEFT)
            Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::START);
          else
            Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::END);
        }
        else
          Schematic::drawTextOnLine(renderer, p1, p2, name(), Schematic::TextLinePos::MIDDLE);
      }

      return;
    }
    else if (ni > 0 && no > 0) {
      Side iside = connections_[0]->inPorts ()[0]->side();
      Side oside = connections_[0]->outPorts()[0]->side();

      bool ilr = (iside == Side::LEFT || iside == Side::RIGHT);
      bool olr = (oside == Side::LEFT || oside == Side::RIGHT);

      QPointF p1, p2;

      if (ilr) {
        if (iside == Side::LEFT) p1 = QPointF(xo1, yoc);
        else                     p1 = QPointF(xo2, yoc);
      }
      else {
        if (iside == Side::TOP ) p1 = QPointF(xoc, yo1);
        else                     p1 = QPointF(xoc, yo1);
      }

      if (olr) {
        if (iside == Side::LEFT) p2 = QPointF(xi1, yic);
        else                     p2 = QPointF(xi2, yic);
      }
      else {
        if (iside == Side::TOP ) p2 = QPointF(xic, yi1);
        else                     p2 = QPointF(xic, yi1);
      }

      Schematic::drawConnection(renderer, p1, p2);

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        Schematic::drawTextAtPoint(renderer,
          QPointF((p1.x() + p2.x())/2, (p1.y() + p2.y())/2), name());
      }

      return;
    }
  }

  //---

  if      (input) {
    Side side = connections_[0]->outPorts()[0]->side();

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

      if (side == Side::LEFT)
        p1 = QPointF(xi1 - dx, yic - n_*dy/2.0);
      else
        p1 = QPointF(xi2 + dx, yic + n_*dy/2.0);
    }
    else {
      if (flipped)
        dx = -dx;

      if (side == Side::TOP)
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

      Schematic::drawLine(renderer, p1 , pm1);
      Schematic::drawLine(renderer, pm1, pm2);
      Schematic::drawLine(renderer, pm2, p2 );

      for (const auto &port : connections_[i]->outPorts())
        Schematic::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        Schematic::drawTextOnLine(renderer, pm2, p2, connections_[i]->name(),
                                  Schematic::TextLinePos::MIDDLE);
      }

      connections_[i]->setPRect(QRectF(p1, p2));

      if (lr) {
        if (side == Side::LEFT)
          p1 += QPointF(0, dy);
        else
          p1 -= QPointF(0, dy);
      }
      else {
        if (side == Side::TOP)
          p1 -= QPointF(dx, 0);
        else
          p1 += QPointF(dx, 0);
      }
    }
  }
  else if (output) {
    Side side = connections_[0]->inPorts()[0]->side();

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

      if (side == Side::LEFT)
        p1 = QPointF(xo1 - dx, yoc + n_*dy/2.0);
      else
        p1 = QPointF(xo2 + dx, yoc - n_*dy/2.0);
    }
    else {
      if (flipped)
        dx = -dx;

      if (side == Side::TOP)
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

      Schematic::drawLine(renderer, p1 , pm1);
      Schematic::drawLine(renderer, pm1, pm2);
      Schematic::drawLine(renderer, pm2, p2 );

      for (const auto &port : connections_[i]->inPorts())
        Schematic::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText()) {
        renderer->painter->setPen(renderer->textColor);

        Schematic::drawTextOnLine(renderer, pm2, p2, connections_[i]->name(),
                                  Schematic::TextLinePos::MIDDLE);
      }

      connections_[i]->setPRect(QRectF(p1, p2));

      if (lr) {
        if (side == Side::LEFT)
          p1 -= QPointF(0, dy);
        else
          p1 += QPointF(0, dy);
      }
      else {
        if (side == Side::TOP)
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

PlacementGroup::
PlacementGroup(const Placement &placement, int nr, int nc) :
 placement_(placement), nr_(nr), nc_(nc)
{
}

PlacementGroup::
~PlacementGroup()
{
  for (auto &placementGroup : placementGroups_)
    delete placementGroup.placementGroup;
}

void
PlacementGroup::
setRect(const QRectF &r)
{
  updateRect();

  double dx = r.left() - rect_.left();
  double dy = r.top () - rect_.top ();

  rect_ = r;

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->setRect(placementGroup->rect().translated(dx, dy));
  }

  for (auto &gateData : gates_) {
    Gate *gate = gateData.gate;

    gate->setRect(gate->rect().translated(dx, dy));
  }
}

void
PlacementGroup::
addGate(Gate *gate, int r, int c, int nr, int nc, Alignment alignment)
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
PlacementGroup::
removeGate(Gate *gate)
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
PlacementGroup::
addConnection(Connection *connection)
{
  connections_.push_back(connection);
}

void
PlacementGroup::
removeConnection(Connection *connection)
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
PlacementGroup::
addBus(Bus *bus)
{
  buses_.push_back(bus);
}

void
PlacementGroup::
removeBus(Bus *bus)
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

PlacementGroup *
PlacementGroup::
addPlacementGroup(const Placement &placement, int nr, int nc, int r1, int c1, int nr1, int nc1,
                  Alignment alignment)
{
  PlacementGroup *placementGroup = new PlacementGroup(placement, nr, nc);

  addPlacementGroup(placementGroup, r1, c1, nr1, nc1, alignment);

  return placementGroup;
}

void
PlacementGroup::
addPlacementGroup(PlacementGroup *placementGroup, int r, int c, int nr, int nc,
                  Alignment alignment)
{
  placementGroups_.push_back(PlacementGroupData(placementGroup, r, c, nr, nc, alignment));

  placementGroup->parentPlacementGroup_ = this;

  rectValid_ = false;
}

PlacementGroup *
PlacementGroup::
replacePlacementGroup(Schematic *schem, PlacementGroup *placementGroup)
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
  PlacementGroup *oldGroup = placementGroups_[i++].placementGroup;

  Gates       oldGates;
  Connections oldConnections;
  Buses       oldBuses;

  oldGroup->hierGates      (oldGates);
  oldGroup->hierConnections(oldConnections);
  oldGroup->hierBuses      (oldBuses);

  //--

  using Names = std::vector<QString>;

  struct ConnectionPorts {
    Names names;
    bool  valid { true };
  };

  using PortConnections = std::map<Connection *,ConnectionPorts>;

  PortConnections portConnections;

  for (auto &gate : oldGates) {
    for (auto &port : gate->inputs()) {
      Connection *connection = port->connection();

      if (connection) {
        ConnectionPorts &connectionPorts = portConnections[connection];

        connectionPorts.names.push_back(port->name());

        connection->removePort(port);
      }
    }

    for (auto &port : gate->outputs()) {
      Connection *connection = port->connection();

      if (connection) {
        ConnectionPorts &connectionPorts = portConnections[connection];

        connectionPorts.names.push_back(port->name());

        connection->removePort(port);
      }
    }

    schem->removeGate(gate);
  }

  //--

  for (auto &portConnection : portConnections) {
    if (! portConnection.second.valid) continue;

    Connection *connection = portConnection.first;

    if (! connection->anyPorts()) {
      schem->removeConnection(connection);

      portConnection.second.valid = false;
    }
  }

  //--

  for (auto &connection : oldConnections) {
    auto p = portConnections.find(connection);
    if (p != portConnections.end()) continue;

    if (! connection->anyPorts())
      schem->removeConnection(connection);
  }

  for (auto &bus : oldBuses)
    schem->removeBus(bus);

  delete oldGroup;

  //---

  for ( ; i < n; ++i)
    placementGroups_[i - 1] = placementGroups_[i];

  placementGroups_.pop_back();

  PlacementGroup *newPlacementGroup = placementGroups_.back().placementGroup;

  //---

  Connections newConnections;

  newPlacementGroup->hierConnections(newConnections);

  for (auto &portConnection : portConnections) {
    if (! portConnection.second.valid) continue;

    Connection *connection = portConnection.first;

    for (auto &newConnection : newConnections) {
      for (const auto &name : portConnection.second.names) {
        if (newConnection->name() == name) {
          newConnection->merge(connection);

          schem->removeConnection(connection);

          portConnection.second.valid = false;

          break;
        }

        if (! portConnection.second.valid)
          break;
      }
    }
  }

  //---

  return newPlacementGroup;
}

void
PlacementGroup::
hierGates(Gates &gates) const
{
  for (auto &gateData : gates_)
    gates.push_back(gateData.gate);

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->hierGates(gates);
  }
}

void
PlacementGroup::
hierConnections(Connections &connections) const
{
  for (auto &connection : connections_)
    connections.push_back(connection);

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->hierConnections(connections);
  }
}

void
PlacementGroup::
hierBuses(Buses &buses) const
{
  for (auto &bus : buses_)
    buses.push_back(bus);

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->hierBuses(buses);
  }
}

void
PlacementGroup::
updateRect() const
{
  if (rectValid_)
    return;

  PlacementGroup *th = const_cast<PlacementGroup *>(this);

  th->rect_ = QRectF();

  for (auto &gateData : gates_) {
    Gate *gate = gateData.gate;

    if (! th->rect_.isEmpty())
      th->rect_ = th->rect_.united(gate->rect());
    else
      th->rect_ = gate->rect();
  }

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    if (! th->rect_.isEmpty())
      th->rect_ = th->rect_.united(placementGroup->rect());
    else
      th->rect_ = placementGroup->rect();
  }

  th->rect_.adjust(-margin_, -margin_, margin_, margin_);
}

QColor
PlacementGroup::
penColor(Renderer *renderer) const
{
  if (renderer->schem->insidePlacement() == this)
    return renderer->insideColor;

  return (isSelected() ? renderer->selectColor : QColor(150,150,250));
}

QSizeF
PlacementGroup::
calcSize() const
{
  const_cast<PlacementGroup *>(this)->place();

  return QSizeF(w_, h_);
}

void
PlacementGroup::
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
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

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
    Gate *gate = gateData.gate;

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

  double x = margin_;
  double y = margin_;

  r = 0;
  c = 0;

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

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
        y1 = margin_;

        for (int r = 0; r < placementGroupData.r; ++r)
          y1 += rowHeights[r];
      }

      if (placementGroupData.c >= 0) {
        x1 = margin_;

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

      if (placementGroupData.alignment == Alignment::HFILL ||
          placementGroupData.alignment == Alignment::FILL)
        w2 = w1;

      if (placementGroupData.alignment == Alignment::VFILL ||
          placementGroupData.alignment == Alignment::FILL)
        h2 = h1;

      rect = QRectF(x1 + (w1 - w2)/2.0, y1 + (h1 - h2)/2.0, w2, h2);

      if (placementGroupData.c < 0) {
        x += colWidths[c];

        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;

          x = margin_;
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
    Gate *gate = gateData.gate;

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
        y1 = margin_;

        for (int r = 0; r < gateData.r; ++r)
          y1 += rowHeights[r];
      }

      if (gateData.c >= 0) {
        x1 = margin_;

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

      if (gateData.alignment == Alignment::HFILL ||
          gateData.alignment == Alignment::FILL)
        w2 = w1;

      if (gateData.alignment == Alignment::VFILL ||
          gateData.alignment == Alignment::FILL)
        h2 = h1;

      rect = QRectF(x1 + (w1 - w2)/2.0, y1 + (h1 - h2)/2.0, w2, h2);

      if (gateData.c < 0) {
        x += colWidths[c];

        ++c;

        if (c >= nc_) {
          ++r;

          c = 0;

          x = margin_;
          y += rowHeights[r];
        }
      }
    }
    else {
      assert(false);
    }

    gate->setRect(rect);

    if (gateData.alignment == Alignment::HFILL ||
        gateData.alignment == Alignment::FILL)
      gate->setWidth(rect.width());

    if (gateData.alignment == Alignment::VFILL ||
        gateData.alignment == Alignment::FILL)
      gate->setHeight(rect.height());
  }

  //---

  w_ += 2*margin_;
  h_ += 2*margin_;

  updateRect();
  //setRect(QRectF(0, 0, w_, h_));
}

void
PlacementGroup::
draw(Renderer *renderer) const
{
  if (! renderer->schem->isPlacementGroupVisible())
    return;

//margin_ = renderer->pixelWidthToWindowWidth(2);

  const_cast<PlacementGroup *>(this)->updateRect();

  renderer->painter->setPen(penColor(renderer));
  renderer->painter->setBrush(Qt::NoBrush);

  prect_ = renderer->windowToPixel(rect());

  renderer->painter->drawRect(prect_);

  for (auto &placementGroupData : placementGroups_) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->draw(renderer);
  }
}

PlacementGroup *
PlacementGroup::
nearestPlacementGroup(const QPointF &p) const
{
  if (! inside(p))
    return nullptr;

  PlacementGroup *minPlacementGroup = const_cast<PlacementGroup *>(this);
  double          minArea           = area();

  for (auto &placementGroupData : placementGroups()) {
    PlacementGroup *placementGroup = placementGroupData.placementGroup;

    PlacementGroup *placementGroup1 = placementGroup->nearestPlacementGroup(p);

    if (placementGroup1) {
      double area = placementGroup1->area();

      if (! minPlacementGroup || area < minArea) {
        minPlacementGroup = placementGroup1;
        minArea           = area;
      }
    }
  }

  return minPlacementGroup;
}

}
