#include <CQSchem.h>
#include <QApplication>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
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

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQSchemWindow *window = new CQSchemWindow;

  CQSchem *schem = window->schem();

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg = &argv[i][1];

      if      (arg == "nand"      ) schem->addNandGate();
      else if (arg == "not"       ) schem->addNotGate();
      else if (arg == "and"       ) schem->addAndGate();
      else if (arg == "and3"      ) schem->addAnd3Gate();
      else if (arg == "and4"      ) schem->addAnd4Gate();
      else if (arg == "and8"      ) schem->addAnd8Gate();
      else if (arg == "or"        ) schem->addOrGate();
      else if (arg == "xor"       ) schem->addXorGate();
      else if (arg == "memory"    ) schem->addMemoryGate();
      else if (arg == "memory8"   ) schem->addMemory8Gate();
      else if (arg == "enabler"   ) schem->addEnablerGate();
      else if (arg == "register"  ) schem->addRegisterGate();
      else if (arg == "decoder4"  ) schem->addDecoder4Gate();
      else if (arg == "decoder8"  ) schem->addDecoder8Gate();
      else if (arg == "decoder16" ) schem->addDecoder16Gate();
      else if (arg == "decoder256") schem->addDecoder256Gate();
      else if (arg == "lshift"    ) schem->addLShiftGate();
      else if (arg == "rshift"    ) schem->addRShiftGate();
      else if (arg == "adder"     ) schem->addAdderGate();

      else if (arg == "build_not"        ) schem->buildNotGate();
      else if (arg == "build_and"        ) schem->buildAndGate();
      else if (arg == "build_and3"       ) schem->buildAnd3Gate();
      else if (arg == "build_and4"       ) schem->buildAnd4Gate();
      else if (arg == "build_and8"       ) schem->buildAnd8Gate();
      else if (arg == "build_or"         ) schem->buildOrGate();
      else if (arg == "build_xor"        ) schem->buildXorGate();
      else if (arg == "build_memory"     ) schem->buildMemoryGate();
      else if (arg == "build_memory8"    ) schem->buildMemory8Gate();
      else if (arg == "build_enabler"    ) schem->buildEnablerGate();
      else if (arg == "build_register"   ) schem->buildRegisterGate();
      else if (arg == "build_decoder4"   ) schem->buildDecoder4Gate();
      else if (arg == "build_decoder8"   ) schem->buildDecoder8Gate();
      else if (arg == "build_decoder16"  ) schem->buildDecoder16Gate();
      else if (arg == "build_decoder256" ) schem->buildDecoder256Gate();
      else if (arg == "build_lshift"     ) schem->buildLShift();
      else if (arg == "build_rshift"     ) schem->buildRShift();
      else if (arg == "build_inverter"   ) schem->buildInverter();
      else if (arg == "build_ander"      ) schem->buildAnder();
      else if (arg == "build_orer"       ) schem->buildOrer();
      else if (arg == "build_xorer"      ) schem->buildXorer();
      else if (arg == "build_adder"      ) schem->buildAdder();
      else if (arg == "build_adder8"     ) schem->buildAdder8();
      else if (arg == "build_comparator" ) schem->buildComparator();
      else if (arg == "build_comparator8") schem->buildComparator8();
      else if (arg == "build_ram256"     ) schem->buildRam256();
      else if (arg == "build_ram65536"   ) schem->buildRam65536();

      else std::cerr << "Invalid arg '-" << arg << "'\n";
    }
  }

  schem->place();

  schem->exec();

  window->show();

  app.exec();
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

  controlLayout->addWidget(connectionTextButton);
  controlLayout->addWidget(gateTextButton);
  controlLayout->addWidget(portTextButton);

  controlLayout->addWidget(moveGateButton);
  controlLayout->addWidget(movePlacementButton);
  controlLayout->addWidget(moveConnectionButton);

  controlLayout->addWidget(connectionVisibleButton);
  controlLayout->addWidget(gateVisibleButton);
  controlLayout->addWidget(placementGroupVisibleButton);

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
  delete placementGroup_;
}

void
CQSchem::
addNandGate()
{
  CQNandGate *gate = addGateT<CQNandGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *out = addConnection("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addNotGate()
{
  CQNotGate *gate = addGateT<CQNotGate>();

  CQConnection *in  = addConnection("a");
  CQConnection *out = addConnection("c");

  gate->connect("a", in );
  gate->connect("c", out);
}

void
CQSchem::
addAndGate()
{
  CQAndGate *gate = addGateT<CQAndGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *out = addConnection("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addAnd3Gate()
{
  CQAnd3Gate *gate = addGateT<CQAnd3Gate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *in3 = addConnection("c");
  CQConnection *out = addConnection("d");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", out);
}

void
CQSchem::
addAnd4Gate()
{
  CQAnd4Gate *gate = addGateT<CQAnd4Gate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *in3 = addConnection("c");
  CQConnection *in4 = addConnection("d");
  CQConnection *out = addConnection("e");

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
  CQAnd8Gate *gate = addGateT<CQAnd8Gate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *in3 = addConnection("c");
  CQConnection *in4 = addConnection("d");
  CQConnection *in5 = addConnection("e");
  CQConnection *in6 = addConnection("f");
  CQConnection *in7 = addConnection("g");
  CQConnection *in8 = addConnection("h");
  CQConnection *out = addConnection("i");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", in3);
  gate->connect("d", in4);
  gate->connect("e", in5);
  gate->connect("f", in6);
  gate->connect("g", in7);
  gate->connect("h", in8);
  gate->connect("e", out);
}

void
CQSchem::
addOrGate()
{
  CQOrGate *gate = addGateT<CQOrGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *out = addConnection("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addXorGate()
{
  CQXorGate *gate = addGateT<CQXorGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *out = addConnection("c");

  gate->connect("a", in1);
  gate->connect("b", in2);
  gate->connect("c", out);
}

void
CQSchem::
addMemoryGate()
{
  CQMemoryGate *gate = addGateT<CQMemoryGate>("M");

  CQConnection *in1 = addConnection("i");
  CQConnection *in2 = addConnection("s");
  CQConnection *out = addConnection("o");

  gate->connect("i", in1);
  gate->connect("s", in2);
  gate->connect("o", out);
}

void
CQSchem::
addMemory8Gate()
{
  CQMemory8Gate *gate = addGateT<CQMemory8Gate>("B");

  CQConnection *cons = addConnection("s");

  gate->connect("s", cons);

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    coni[i] = addConnection(iname);
    cono[i] = addConnection(oname);

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
  CQEnablerGate *gate = addGateT<CQEnablerGate>("E");

  CQConnection *cone = addConnection("e");

  gate->connect("e", cone);

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQEnablerGate::iname(i);
    QString oname = CQEnablerGate::oname(i);

    coni[i] = addConnection(iname);
    cono[i] = addConnection(oname);

    gate->connect(iname, coni[i]);
    gate->connect(oname, cono[i]);

    ibus->addConnection(coni[i], i);
    obus->addConnection(cono[i], i);
  }
}

void
CQSchem::
addRegisterGate()
{
  CQRegisterGate *gate = addGateT<CQRegisterGate>("R");

  gate->connect("s", addConnection("s"));
  gate->connect("e", addConnection("e"));

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *coni[8];
  CQConnection *cono[8];

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    coni[i] = addConnection(iname);
    cono[i] = addConnection(oname);

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
  CQDecoder4Gate *gate = addGateT<CQDecoder4Gate>("2x4");

  for (int i = 0; i < 2; ++i) {
    QString iname = CQDecoder4Gate::iname(i);

    CQConnection *con = addConnection(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 4; ++i) {
    QString oname = CQDecoder4Gate::oname(i);

    CQConnection *cono = addConnection(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder8Gate()
{
  CQDecoder8Gate *gate = addGateT<CQDecoder8Gate>("3x8");

  for (int i = 0; i < 3; ++i) {
    QString iname = CQDecoder8Gate::iname(i);

    CQConnection *con = addConnection(iname);

    gate->connect(iname, con);
  }

  for (int i = 0; i < 8; ++i) {
    QString oname = CQDecoder8Gate::oname(i);

    CQConnection *cono = addConnection(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder16Gate()
{
  CQDecoder16Gate *gate = addGateT<CQDecoder16Gate>("4x16");

  for (int i = 0; i < 4; ++i) {
    QString iname = CQDecoder16Gate::iname(i);

    CQConnection *coni = addConnection(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 16; ++i) {
    QString oname = CQDecoder16Gate::oname(i);

    CQConnection *cono = addConnection(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addDecoder256Gate()
{
  CQDecoder256Gate *gate = addGateT<CQDecoder256Gate>("8x256");

  for (int i = 0; i < 8; ++i) {
    QString iname = CQDecoder256Gate::iname(i);

    CQConnection *coni = addConnection(iname);

    gate->connect(iname, coni);
  }

  for (int i = 0; i < 256; ++i) {
    QString oname = CQDecoder256Gate::oname(i);

    CQConnection *cono = addConnection(oname);

    gate->connect(oname, cono);
  }
}

void
CQSchem::
addLShiftGate()
{
  CQLShiftGate *gate = addGateT<CQLShiftGate>("SHL");

  gate->connect("s", addConnection("s"));
  gate->connect("e", addConnection("e"));

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *icon[8];
  CQConnection *ocon[8];

  gate->connect("shift_in" , addConnection("shift_in" ));
  gate->connect("shift_out", addConnection("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addConnection(CQLShiftGate::iname(i));
    ocon[i] = addConnection(CQLShiftGate::oname(i));

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
  CQRShiftGate *gate = addGateT<CQRShiftGate>("SHR");

  gate->connect("s", addConnection("s"));
  gate->connect("e", addConnection("e"));

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *icon[8];
  CQConnection *ocon[8];

  gate->connect("shift_in" , addConnection("shift_in" ));
  gate->connect("shift_out", addConnection("shift_out"));

  for (int i = 0; i < 8; ++i) {
    icon[i] = addConnection(CQRShiftGate::iname(i));
    ocon[i] = addConnection(CQRShiftGate::oname(i));

    gate->connect(CQRShiftGate::iname(i), icon[i]);
    gate->connect(CQRShiftGate::oname(i), ocon[i]);

    ibus->addConnection(icon[i], i);
    obus->addConnection(ocon[i], i);
  }
}

void
CQSchem::
addAdderGate()
{
  CQAdderGate *gate = addGateT<CQAdderGate>("adder");

  gate->connect("a"        , addConnection("a"        ));
  gate->connect("b"        , addConnection("b"        ));
  gate->connect("carry_in" , addConnection("carry_in" ));
  gate->connect("carry_out", addConnection("carry_out"));
  gate->connect("sum"      , addConnection("sum"      ));
}

void
CQSchem::
buildNotGate()
{
  CQNandGate *gate = addGateT<CQNandGate>();

  CQConnection *in  = addConnection("a");
  CQConnection *out = addConnection("c");

  gate->connect("a", in );
  gate->connect("b", in );
  gate->connect("c", out);
}

void
CQSchem::
buildAndGate()
{
  CQNandGate *nandGate = addGateT<CQNandGate>();

  CQConnection *in1  = addConnection("a");
  CQConnection *in2  = addConnection("b");
  CQConnection *out1 = addConnection("x");

  nandGate->connect("a", in1 );
  nandGate->connect("b", in2 );
  nandGate->connect("c", out1);

  CQNotGate *notGate = addGateT<CQNotGate>();

  CQConnection *out2 = addConnection("c");

  notGate->connect("a", out1);
  notGate->connect("c", out2);
}

void
CQSchem::
buildAnd3Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 2);

  CQAndGate *andGate1 = addGateT<CQAndGate>();
  CQAndGate *andGate2 = addGateT<CQAndGate>();

  CQConnection *in1  = addConnection("a");
  CQConnection *in2  = addConnection("b");
  CQConnection *in3  = addConnection("c");
  CQConnection *out1 = addConnection("t");
  CQConnection *out2 = addConnection("d");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);

  placementGroup->addGate(andGate1, 1, 0);
  placementGroup->addGate(andGate2, 0, 1);
}

void
CQSchem::
buildAnd4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  CQAndGate *andGate1 = addGateT<CQAndGate>();
  CQAndGate *andGate2 = addGateT<CQAndGate>();
  CQAndGate *andGate3 = addGateT<CQAndGate>();

  CQConnection *in1  = addConnection("a");
  CQConnection *in2  = addConnection("b");
  CQConnection *in3  = addConnection("c");
  CQConnection *in4  = addConnection("d");
  CQConnection *out1 = addConnection("t1");
  CQConnection *out2 = addConnection("t2");
  CQConnection *out3 = addConnection("e");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);

  andGate3->connect("a", out2);
  andGate3->connect("b", in4 );
  andGate3->connect("c", out3);

  placementGroup->addGate(andGate1, 2, 0);
  placementGroup->addGate(andGate2, 1, 1);
  placementGroup->addGate(andGate3, 0, 2);
}

void
CQSchem::
buildAnd8Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 7, 7);

  CQAndGate *andGate1 = addGateT<CQAndGate>();
  CQAndGate *andGate2 = addGateT<CQAndGate>();
  CQAndGate *andGate3 = addGateT<CQAndGate>();
  CQAndGate *andGate4 = addGateT<CQAndGate>();
  CQAndGate *andGate5 = addGateT<CQAndGate>();
  CQAndGate *andGate6 = addGateT<CQAndGate>();
  CQAndGate *andGate7 = addGateT<CQAndGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *in3 = addConnection("c");
  CQConnection *in4 = addConnection("d");
  CQConnection *in5 = addConnection("e");
  CQConnection *in6 = addConnection("f");
  CQConnection *in7 = addConnection("g");
  CQConnection *in8 = addConnection("h");

  CQConnection *out1 = addConnection("t1");
  CQConnection *out2 = addConnection("t2");
  CQConnection *out3 = addConnection("t3");
  CQConnection *out4 = addConnection("t4");
  CQConnection *out5 = addConnection("t5");
  CQConnection *out6 = addConnection("t6");
  CQConnection *out7 = addConnection("e");

  andGate1->connect("a", in1 );
  andGate1->connect("b", in2 );
  andGate1->connect("c", out1);

  andGate2->connect("a", out1);
  andGate2->connect("b", in3 );
  andGate2->connect("c", out2);

  andGate3->connect("a", out2);
  andGate3->connect("b", in4 );
  andGate3->connect("c", out3);

  andGate4->connect("a", out3);
  andGate4->connect("b", in5 );
  andGate4->connect("c", out4);

  andGate5->connect("a", out4);
  andGate5->connect("b", in6 );
  andGate5->connect("c", out5);

  andGate6->connect("a", out5);
  andGate6->connect("b", in7 );
  andGate6->connect("c", out6);

  andGate7->connect("a", out6);
  andGate7->connect("b", in8 );
  andGate7->connect("c", out7);

  placementGroup->addGate(andGate1, 6, 0);
  placementGroup->addGate(andGate2, 5, 1);
  placementGroup->addGate(andGate3, 4, 2);
  placementGroup->addGate(andGate4, 3, 3);
  placementGroup->addGate(andGate5, 2, 4);
  placementGroup->addGate(andGate6, 1, 5);
  placementGroup->addGate(andGate7, 0, 6);
}

void
CQSchem::
buildOrGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 2);

  CQNotGate  *notGate1 = addGateT<CQNotGate>();
  CQNotGate  *notGate2 = addGateT<CQNotGate>();
  CQNandGate *nandGate = addGateT<CQNandGate>();

  CQConnection *in1 = addConnection("a");
  CQConnection *in2 = addConnection("b");
  CQConnection *io1 = addConnection("c");
  CQConnection *io2 = addConnection("d");
  CQConnection *out = addConnection("e");

  notGate1->connect("a", in1);
  notGate1->connect("c", io1);

  notGate2->connect("a", in2);
  notGate2->connect("c", io2);

  nandGate->connect("a", io1);
  nandGate->connect("b", io2);
  nandGate->connect("c", out);

  placementGroup->addGate(notGate1, 1, 0);
  placementGroup->addGate(notGate2, 0, 0);
  placementGroup->addGate(nandGate, 0, 1, 2, 1);
}

void
CQSchem::
buildXorGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  CQNotGate  *notGate1  = addGateT<CQNotGate >("not1");
  CQNotGate  *notGate2  = addGateT<CQNotGate >("not2");
  CQNandGate *nandGate1 = addGateT<CQNandGate>("nand1");
  CQNandGate *nandGate2 = addGateT<CQNandGate>("nand2");
  CQNandGate *nandGate3 = addGateT<CQNandGate>("nand3");

  CQConnection *acon = addConnection("a");
  CQConnection *bcon = addConnection("b");
  CQConnection *ccon = addConnection("c");
  CQConnection *dcon = addConnection("d");
  CQConnection *econ = addConnection("e");
  CQConnection *fcon = addConnection("f");
  CQConnection *gcon = addConnection("g");

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

  placementGroup->addGate(notGate1 , 1, 0);
  placementGroup->addGate(notGate2 , 0, 0);
  placementGroup->addGate(nandGate1, 1, 1);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 0, 2, 2, 1);
}

void
CQSchem::
buildMemoryGate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

  CQNandGate *nandGate1 = addGateT<CQNandGate>("1");
  CQNandGate *nandGate2 = addGateT<CQNandGate>("2");
  CQNandGate *nandGate3 = addGateT<CQNandGate>("3");
  CQNandGate *nandGate4 = addGateT<CQNandGate>("4");

  CQConnection *coni = addConnection("i");
  CQConnection *cons = addConnection("s");
  CQConnection *cona = addConnection("a");
  CQConnection *conb = addConnection("b");
  CQConnection *conc = addConnection("c");
  CQConnection *cono = addConnection("o");

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

  placementGroup->addGate(nandGate1, 1, 0);
  placementGroup->addGate(nandGate2, 0, 1);
  placementGroup->addGate(nandGate3, 1, 2);
  placementGroup->addGate(nandGate4, 0, 2);
}

void
CQSchem::
buildMemory8Gate()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *cons = addConnection("s");

  CQMemoryGate *mem[8];

  for (int i = 0; i < 8; ++i) {
    QString memname = QString("mem%1").arg(i);

    mem[i] = addGateT<CQMemoryGate>(memname);

    mem[i]->setSSide(CQPort::Side::BOTTOM);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

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
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  CQConnection *cone = addConnection("e");

  CQAndGate *gate[8];

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    gate[i] = addGateT<CQAndGate>(andname);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

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
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::HORIZONTAL);

  //---

  CQMemory8Gate *mem = addGateT<CQMemory8Gate>("B");
  CQEnablerGate *ena = addGateT<CQEnablerGate>("E");

  CQConnection *cons = addConnection("s");
  CQConnection *cone = addConnection("e");

  mem->connect("s", cons);
  ena->connect("e", cone);

  CQBus *ibus  = addBus( "i", 8);
  CQBus *iobus = addBus("io", 8);
  CQBus *obus  = addBus( "o", 8);

  for (int i = 0; i < 8; ++i) {
    QString ioname = QString("io%1").arg(i);

    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    CQConnection *icon  = addConnection(iname);
    CQConnection *iocon = addConnection(ioname);
    CQConnection *ocon  = addConnection(oname);

    mem->connect(iname, icon);
    mem->connect(oname, iocon);

    ena->connect(iname, iocon);
    ena->connect(oname, ocon);

    ibus ->addConnection(icon , i);
    iobus->addConnection(iocon, i);
    obus ->addConnection(ocon , i);
  }

  placementGroup->addGate(mem);
  placementGroup->addGate(ena);
}

void
CQSchem::
buildDecoder4Gate()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 5, 3);

  //---

  CQConnection *cona = addConnection("a");
  CQConnection *conb = addConnection("b");

  CQConnection *conna = addConnection("na");
  CQConnection *connb = addConnection("nb");

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

    CQConnection *out = addConnection(oname);

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

  //---

  CQConnection *cona = addConnection("a");
  CQConnection *conb = addConnection("b");
  CQConnection *conc = addConnection("c");

  CQConnection *conna = addConnection("na");
  CQConnection *connb = addConnection("nb");
  CQConnection *connc = addConnection("nc");

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

    CQConnection *out = addConnection(oname);

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

  //---

  CQConnection *cona = addConnection("a");
  CQConnection *conb = addConnection("b");
  CQConnection *conc = addConnection("c");
  CQConnection *cond = addConnection("d");

  CQConnection *conna = addConnection("na");
  CQConnection *connb = addConnection("nb");
  CQConnection *connc = addConnection("nc");
  CQConnection *connd = addConnection("nd");

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

    CQConnection *out = addConnection(outname);

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

  //---

  CQConnection *cona = addConnection("a");
  CQConnection *conb = addConnection("b");
  CQConnection *conc = addConnection("c");
  CQConnection *cond = addConnection("d");
  CQConnection *cone = addConnection("e");
  CQConnection *conf = addConnection("f");
  CQConnection *cong = addConnection("g");
  CQConnection *conh = addConnection("h");

  CQConnection *conna = addConnection("na");
  CQConnection *connb = addConnection("nb");
  CQConnection *connc = addConnection("nc");
  CQConnection *connd = addConnection("nd");
  CQConnection *conne = addConnection("ne");
  CQConnection *connf = addConnection("nf");
  CQConnection *conng = addConnection("ng");
  CQConnection *connh = addConnection("nh");

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

    CQConnection *out = addConnection(outname);

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
  CQRegisterGate *gate1 = addGateT<CQRegisterGate>("R1");
  CQRegisterGate *gate2 = addGateT<CQRegisterGate>("R2");

  CQConnection *iocon[9];

  iocon[0] = addConnection("shift_out");
  iocon[8] = addConnection("shift_in");

  for (int i = 1; i <= 7; ++i)
    iocon[i] = addConnection(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *icon1 = addConnection(iname);
    CQConnection *ocon2 = addConnection(oname);

    gate1->connect(iname, icon1);
    gate1->connect(oname, iocon[i]);

    gate2->connect(iname, iocon[i + 1]);
    gate2->connect(oname, ocon2);
  }

  //---

  gate1->connect("s", addConnection("s1"));
  gate1->connect("e", addConnection("e1"));

  gate2->connect("s", addConnection("s2"));
  gate2->connect("e", addConnection("e2"));
}

void
CQSchem::
buildRShift()
{
  CQRegisterGate *gate1 = addGateT<CQRegisterGate>("R1");
  CQRegisterGate *gate2 = addGateT<CQRegisterGate>("R2");

  CQConnection *iocon[9];

  iocon[0] = addConnection("shift_out");
  iocon[8] = addConnection("shift_in");

  for (int i = 0; i < 7; ++i)
    iocon[i + 1] = addConnection(QString("io%1").arg(i));

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *icon1 = addConnection(iname);
    CQConnection *ocon2 = addConnection(oname);

    gate1->connect(iname, icon1);
    gate1->connect(oname, iocon[i]);

    gate2->connect(iname, iocon[i + 1]);
    gate2->connect(oname, ocon2);
  }

  //---

  gate1->connect("s", addConnection("s1"));
  gate1->connect("e", addConnection("e1"));

  gate2->connect("s", addConnection("s2"));
  gate2->connect("e", addConnection("e2"));
}

void
CQSchem::
buildInverter()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus = addBus("i", 8);
  CQBus *obus = addBus("o", 8);

  for (int i = 0; i < 8; ++i) {
    QString name = QString("not%1").arg(i);

    CQNotGate *gate = addGateT<CQNotGate>(name);

    QString iname = QString("a%1").arg(i);
    QString oname = QString("c%1").arg(i);

    CQConnection *icon = addConnection(iname);
    CQConnection *ocon = addConnection(oname);

    gate->connect("a", icon);
    gate->connect("c", ocon);

    ibus->addConnection(icon, i);
    obus->addConnection(ocon, i);

    placementGroup->addGate(gate);
  }
}

void
CQSchem::
buildAnder()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus1 = addBus("i1", 8);
  CQBus *ibus2 = addBus("i2", 8);
  CQBus *obus  = addBus("o" , 8);

  for (int i = 0; i < 8; ++i) {
    QString name = QString("and%1").arg(i);

    CQAndGate *gate = addGateT<CQAndGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    CQConnection *icon1 = addConnection(iname1);
    CQConnection *icon2 = addConnection(iname2);
    CQConnection *ocon  = addConnection(oname);

    gate->connect("a", icon1);
    gate->connect("b", icon2);
    gate->connect("c", ocon );

    ibus1->addConnection(icon1, i);
    ibus2->addConnection(icon2, i);
    obus ->addConnection(ocon , i);

    placementGroup->addGate(gate);
  }
}

void
CQSchem::
buildOrer()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus1 = addBus("i1", 8);
  CQBus *ibus2 = addBus("i2", 8);
  CQBus *obus  = addBus("o" , 8);

  for (int i = 0; i < 8; ++i) {
    QString name = QString("or%1").arg(i);

    CQOrGate *gate = addGateT<CQOrGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    CQConnection *icon1 = addConnection(iname1);
    CQConnection *icon2 = addConnection(iname2);
    CQConnection *ocon  = addConnection(oname);

    gate->connect("a", icon1);
    gate->connect("b", icon2);
    gate->connect("c", ocon );

    ibus1->addConnection(icon1, i);
    ibus2->addConnection(icon2, i);
    obus ->addConnection(ocon , i);

    placementGroup->addGate(gate);
  }
}

void
CQSchem::
buildXorer()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus1 = addBus("i1", 8);
  CQBus *ibus2 = addBus("i2", 8);
  CQBus *obus  = addBus("o" , 8);

  for (int i = 0; i < 8; ++i) {
    QString name = QString("xor%1").arg(i);

    CQXorGate *gate = addGateT<CQXorGate>(name);

    QString iname1 = QString("a%1").arg(i);
    QString iname2 = QString("b%1").arg(i);
    QString oname  = QString("c%1").arg(i);

    CQConnection *icon1 = addConnection(iname1);
    CQConnection *icon2 = addConnection(iname2);
    CQConnection *ocon  = addConnection(oname);

    gate->connect("a", icon1);
    gate->connect("b", icon2);
    gate->connect("c", ocon );

    ibus1->addConnection(icon1, i);
    ibus2->addConnection(icon2, i);
    obus ->addConnection(ocon , i);

    placementGroup->addGate(gate);
  }
}

void
CQSchem::
buildAdder()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 3, 3);

  //---

  CQXorGate *xorGate1 = addGateT<CQXorGate>("xor1");
  CQXorGate *xorGate2 = addGateT<CQXorGate>("xor2");
  CQAndGate *andGate1 = addGateT<CQAndGate>("and1");
  CQAndGate *andGate2 = addGateT<CQAndGate>("and2");
  CQOrGate  *orGate   = addGateT<CQOrGate >("or"  );

  CQConnection *acon  = addConnection("a");
  CQConnection *bcon  = addConnection("b");
  CQConnection *cconi = addConnection("carry_in");
  CQConnection *scon  = addConnection("sum");
  CQConnection *ccono = addConnection("carry_out");

  CQConnection *scon1 = addConnection("");
  CQConnection *ccon1 = addConnection("");
  CQConnection *ccon2 = addConnection("");

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

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(xorGate2, 2, 2);
  placementGroup->addGate(andGate1, 1, 1);
  placementGroup->addGate(andGate2, 0, 1);
  placementGroup->addGate(orGate  , 0, 2, 2, 1);
}

void
CQSchem::
buildAdder8()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQBus *ibus = addBus("i"  , 8); ibus->setPosition(CQBus::Position::START);
  CQBus *obus = addBus("o"  , 8); obus->setPosition(CQBus::Position::END);
  CQBus *sbus = addBus("sum", 8);

  CQAdderGate  *adders[8];
  CQConnection *acon  [8];
  CQConnection *bcon  [8];
  CQConnection *scon  [8];

  CQConnection *cicon = nullptr;
  CQConnection *cocon = nullptr;

  for (int i = 0; i < 8; ++i) {
    adders[i] = addGateT<CQAdderGate>(QString("adder%1").arg(i));

    acon[i] = addConnection(QString("a%1").arg(i));
    bcon[i] = addConnection(QString("b%1").arg(i));
    scon[i] = addConnection(QString("sum%1").arg(i));

    adders[i]->connect("a", acon[i]);
    adders[i]->connect("b", bcon[i]);

    if (i == 0)
      cicon = addConnection("carry_in");

    cocon = addConnection("carry_out");

    adders[i]->connect("carry_in" , cicon);
    adders[i]->connect("carry_out", cocon);

    cicon = cocon;

    adders[i]->connect("sum", scon[i]);

    ibus->addConnection(acon[i], i);
    obus->addConnection(bcon[i], i);
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

  //---

  CQXorGate  *xorGate1 = addGateT<CQXorGate >("xor1");
  CQNotGate  *notGate2 = addGateT<CQNotGate >("not2");
  CQAndGate  *andGate3 = addGateT<CQAndGate >("and3");
  CQAnd3Gate *andGate4 = addGateT<CQAnd3Gate>("and4");
  CQOrGate   *orGate5  = addGateT<CQOrGate  >("or5" );

  CQConnection *acon = addConnection("a");
  CQConnection *bcon = addConnection("b");
  CQConnection *ccon = addConnection("c");

  CQConnection *equalCon     = addConnection("equal");
  CQConnection *iallEqualCon = addConnection("i_all_equal");
  CQConnection *oallEqualCon = addConnection("o_all_equal");

  CQConnection *dcon = addConnection("d");

  CQConnection *iaLargerCon = addConnection("i_a_larger");
  CQConnection *oaLargerCon = addConnection("o_a_larger");

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

  placementGroup->addGate(xorGate1, 2, 0);
  placementGroup->addGate(notGate2, 1, 1);
  placementGroup->addGate(andGate3, 0, 2);
  placementGroup->addGate(andGate4, 3, 3);
  placementGroup->addGate(orGate5 , 0, 4);
}

void
CQSchem::
buildComparator8()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  CQConnection *iallEqualCon = addConnection("i_all_equal");
  CQConnection *iaLargerCon  = addConnection("i_a_larger" );

  iallEqualCon->setValue(1);

  CQBus *abus = addBus("a", 8); abus->setPosition(CQBus::Position::START);
  CQBus *bbus = addBus("b", 8); bbus->setPosition(CQBus::Position::END);
  CQBus *cbus = addBus("c", 8);

  CQConnection *acon[8];
  CQConnection *bcon[8];
  CQConnection *ccon[8];

  for (int i = 0; i < 8; ++i) {
    CQPlacementGroup *placementGroup1 =
      placementGroup->addPlacementGroup(CQPlacementGroup::Placement::GRID, 4, 5);

    //---

    CQXorGate  *xorGate1 = addGateT<CQXorGate >("xor1");
    CQNotGate  *notGate2 = addGateT<CQNotGate >("not2");
    CQAndGate  *andGate3 = addGateT<CQAndGate >("and3");
    CQAnd3Gate *andGate4 = addGateT<CQAnd3Gate>("and4");
    CQOrGate   *orGate5  = addGateT<CQOrGate  >("or5" );

    acon[i] = addConnection("a");
    bcon[i] = addConnection("b");
    ccon[i] = addConnection("c");

    CQConnection *equalCon = addConnection("equal");

    CQConnection *dcon = addConnection("d");

    CQConnection *oallEqualCon = addConnection("o_all_equal");
    CQConnection *oaLargerCon  = addConnection("o_a_larger" );

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

    placementGroup1->addGate(xorGate1, 1, 0);
    placementGroup1->addGate(notGate2, 2, 1);
    placementGroup1->addGate(andGate3, 3, 2);
    placementGroup1->addGate(andGate4, 0, 4);
    placementGroup1->addGate(orGate5 , 3, 4);

    abus->addConnection(acon[i], i);
    bbus->addConnection(bcon[i], i);
    cbus->addConnection(ccon[i], i);

    iallEqualCon = oallEqualCon;
    iaLargerCon  = oaLargerCon;
  }
}

void
CQSchem::
buildRam256()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

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

  CQBus *ibus  = addBus("i" , 8);
  CQBus *obus1 = addBus("o1", 4);
  CQBus *obus2 = addBus("o2", 4);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *in  = addConnection(iname);
    CQConnection *out = addConnection(oname);

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

    hout[i] = addConnection(honame);
    vout[i] = addConnection(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate->connect("s", addConnection("sa"));

  CQConnection *s = addConnection("s");
  CQConnection *e = addConnection("e");

  //---

  CQConnection *bus[8];

  CQBus *iobus = addBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addConnection(QString("bus[%1]").arg(i));

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

      CQConnection *t1 = addConnection("t1");

      xgate->connect("c", t1);

      CQAndGate *agate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));
      CQAndGate *bgate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));

      CQRegisterGate *rgate = addGateT<CQRegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      CQConnection *t2 = addConnection("t2");
      CQConnection *t3 = addConnection("t3");

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

  CQBus *ibus  = addBus("i" , 8);
  CQBus *obus1 = addBus("o1", 8);
  CQBus *obus2 = addBus("o2", 8);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *in   = addConnection(iname);
    CQConnection *out1 = addConnection(oname);
    CQConnection *out2 = addConnection(oname);

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

    hout[i] = addConnection(honame);
    vout[i] = addConnection(voname);

    hdec->connect(honame, hout[i]);
    vdec->connect(voname, vout[i]);
  }

  rgate0->connect("s", addConnection("s0"));
  rgate1->connect("s", addConnection("s1"));

  CQConnection *s = addConnection("s");
  CQConnection *e = addConnection("e");

  //---

  CQConnection *bus[8];

  CQBus *iobus = addBus("io", 8);

  for (int i = 0; i < 8; ++i) {
    bus[i] = addConnection(QString("bus[%1]").arg(i));

    iobus->addConnection(bus[i], i);
  }

  //---

  CQPlacementGroup *placementGroup1 =
    new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 256, 256);

  for (int r = 0; r < 256; ++r) {
std::cerr << "Row: " << r << "\n";
    for (int c = 0; c < 256; ++c) {
      CQPlacementGroup *placementGroup2 =
        new CQPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 3);

      //---

      CQAndGate *xgate = addGateT<CQAndGate>(QString("X_%1_%2").arg(r).arg(c));

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      CQConnection *t1 = addConnection("t1");

      xgate->connect("c", t1);

      CQAndGate *agate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));
      CQAndGate *bgate = addGateT<CQAndGate>(QString("X1_%1_%2").arg(r).arg(c));

      CQRegisterGate *rgate = addGateT<CQRegisterGate>(QString("R_%1_%2").arg(r).arg(c));

      CQConnection *t2 = addConnection("t2");
      CQConnection *t3 = addConnection("t3");

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
addGate(CQGate *gate)
{
  gates_.push_back(gate);

  placementGroup_->addGate(gate);
}

CQBus *
CQSchem::
addBus(const QString &name, int n)
{
  CQBus *bus = new CQBus(name, n);

  buses_.push_back(bus);

  return bus;
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

  renderer_.schem   = this;
  renderer_.painter = &painter;
  renderer_.prect   = rect();
  renderer_.rect    = rect_;

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
  if      (e->key() ==  Qt::Key_Plus)
    renderer_.displayTransform.zoomIn();
  else if (e->key() ==  Qt::Key_Minus)
    renderer_.displayTransform.zoomOut();
  else if (e->key() ==  Qt::Key_Left)
    renderer_.displayTransform.panLeft();
  else if (e->key() ==  Qt::Key_Right)
    renderer_.displayTransform.panRight();
  else if (e->key() ==  Qt::Key_Up)
    renderer_.displayTransform.panUp();
  else if (e->key() ==  Qt::Key_Down)
    renderer_.displayTransform.panDown();
  else if (e->key() ==  Qt::Key_Home)
    renderer_.displayTransform.reset();
  else if (e->key() ==  Qt::Key_Greater) {
    Gates gates;

    selectedGates(gates);

    for (auto &gate : gates)
      gate->nextOrient();
  }
  else if (e->key() ==  Qt::Key_Less) {
    Gates gates;

    selectedGates(gates);

    for (auto &gate : gates)
      gate->prevOrient();
  }

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

      renderer->painter->drawLine(p1                 , QPointF(p1.x(), ym));
      renderer->painter->drawLine(QPointF(p1.x(), ym), QPointF(p2.x(), ym));
      renderer->painter->drawLine(QPointF(p2.x(), ym), p2                 );
    }
    else
      renderer->painter->drawLine(p1, p2);
  }
  else {
    if (dx > 1E-6) {
      double xm = (p1.x() + p2.x())/2.0;

      renderer->painter->drawLine(p1                 , QPointF(xm, p1.y()));
      renderer->painter->drawLine(QPointF(xm, p1.y()), QPointF(xm, p2.y()));
      renderer->painter->drawLine(QPointF(xm, p2.y()), p2                 );
    }
    else
      renderer->painter->drawLine(p1, p2);
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
drawTextAtPoint(CQSchemRenderer *renderer, const QPointF &p, const QString &text)
{
  double fh = renderer->windowHeightToPixelHeight(0.25);
  if (fh < 3) return;

  renderer->setFontSize(fh);

  QFontMetricsF fm(renderer->painter->font());

  double dx = fm.width(text)/2.0;
  double dy = (fm.ascent() - fm.descent())/2.0;

  QPointF pm(p.x() - dx, p.y() + dy);

  renderer->painter->drawText(pm, text);
}

//---

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

  if (renderer->schem->isShowGateText())
    CQSchem::drawTextInRect(renderer, prect_, name());

//renderer->painter->setPen(Qt::red);
//renderer->painter->drawRect(prect_);

  if (renderer->schem->isShowPortText()) {
    for (auto &port : inputs())
      CQSchem::drawTextAtPoint(renderer, port->pixelPos(), port->name());

    for (auto &port : outputs())
      CQSchem::drawTextAtPoint(renderer, port->pixelPos(), port->name());
  }
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

  renderer->painter->strokePath(path, renderer->painter->pen());
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

    path.moveTo(x1, y1);
    path.lineTo(xm, y1);
    path.quadTo(x2, y1, x2, ym);
    path.quadTo(x2, y2, xm, y2);
    path.lineTo(x1, y2);
    path.quadTo(x3, ym, x1, y1);
  }
  else if (orientation() == Orientation::R90) {
    y3 = y1 + (y2 - y1)/4.0;
    x3 = x1;

    path.moveTo(x2, y1);
    path.lineTo(x2, ym);
    path.quadTo(x2, y2, xm, y2);
    path.quadTo(x1, y2, x1, ym);
    path.lineTo(x1, y1);
    path.quadTo(xm, y3, x2, y1);
  }
  else if (orientation() == Orientation::R180) {
    x3 = x2 - (x2 - x1)/4.0;
    y3 = y2;

    path.moveTo(x2, y2);
    path.lineTo(xm, y2);
    path.quadTo(x1, y2, x1, ym);
    path.quadTo(x1, y1, xm, y1);
    path.lineTo(x2, y1);
    path.quadTo(x3, ym, x2, y2);
  }
  else if (orientation() == Orientation::R270) {
    y3 = y2 - (y2 - y1)/4.0;
    x3 = x2;

    path.moveTo(x1, y2);
    path.lineTo(x1, ym);
    path.quadTo(x1, y1, xm, y1);
    path.quadTo(x2, y1, x2, ym);
    path.lineTo(x2, y2);
    path.quadTo(xm, y3, x1, y2);
  }

  path.closeSubpath();

  renderer->painter->strokePath(path, renderer->painter->pen());
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

  renderer->painter->strokePath(path, renderer->painter->pen());
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
    return Qt::cyan;

  return (isSelected() ? Qt::yellow : Qt::white);
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
  drawAnd(renderer, px1(), py1(), px2(), py2());

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
  drawAnd(renderer, px1(), py1(), px2(), py2());

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
  drawAnd(renderer, px1(), py1(), px2(), py2());

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
  drawAnd(renderer, px1(), py1(), px2(), py2());

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
  addInputPorts(QStringList() << "a" << "b" << "c" << "d" << "e" << "f" << "g" << "h");

  addOutputPort("i");
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
  drawAnd(renderer, px1(), py1(), px2(), py2());

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
  double x3, y3;

  drawOr(renderer, px1(), py1(), px2(), py2(), x3, y3);

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
  drawXor(renderer, px1(), py1(), px2(), py2());

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

  //---

  // place ports and draw connections
  placePorts(8, 8);

  // place port s on bottom
  placePortOnSide(inputs_[8], CQPort::Side::BOTTOM);

  //---

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

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
  renderer->painter->drawRect(prect_);

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

  addOutputPorts(QStringList() << "carry_out" << "sum");
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

  if (co != outputs_[0]->getValue() || sum != outputs_[1]->getValue()) {
    outputs_[0]->setValue(co);
    outputs_[1]->setValue(sum);

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

  auto mapX = [&](double x) {
    return renderer->windowToPixel(QPointF(x*rect_.width () + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) {
    return renderer->windowToPixel(QPointF(0.0, y*rect_.height() + rect_.y())).y(); };

  //---

  // draw gate
  double x1 = mapX(xmargin());
  double x2 = mapX(1.0 - xmargin());
  double y1 = mapY(ymargin());
  double y2 = mapY(1.0 - ymargin());

  prect_ = QRectF(x1, y2, x2 - x1, y1 - y2);

  double ym = (y1 + y2)/2.0;
  double xm = (x1 + x2)/2.0;

  double ym1 = (y1 + ym)/2.0;
  double ym2 = (y2 + ym)/2.0;

  double xm1 = (x1 + xm)/2.0;

  QPainterPath path;

  path.moveTo(x1 , y1 );
  path.lineTo(x1 , ym1);
  path.lineTo(xm1, ym );
  path.lineTo(x1 , ym2);
  path.lineTo(x1 , y2);
  path.lineTo(x2 , ym2);
  path.lineTo(x2 , ym1);

  path.closeSubpath();

  renderer->painter->strokePath(path, renderer->painter->pen());

  //renderer->painter->drawRect(prect_);

  //---

  // draw input connections (a, b, carry_in)
  CQPort *cona  = getPortByName("a");
  CQPort *conb  = getPortByName("b");
  CQPort *conci = getPortByName("carry_in");

  cona ->setPixelPos(QPointF(x1, (y1 + ym1)/2));
  conb ->setPixelPos(QPointF(x1, (y2 + ym2)/2));
  conci->setPixelPos(QPointF(xm, (y2 + ym2)/2)); conci->setSide(CQPort::Side::BOTTOM);

  // draw output connections (carry_out, sum)
  CQPort *conco = getPortByName("carry_out");
  CQPort *cons  = getPortByName("sum");

  conco->setPixelPos(QPointF(xm, (y1 + ym1)/2)); conco->setSide(CQPort::Side::TOP);
  cons ->setPixelPos(QPointF(x2, ym));

  //---

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

  renderer->painter->strokePath(path, renderer->painter->pen());

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

  addInputPort("shift_in");

  for (int i = 0; i < 8; ++i) {
    addInputPort (iname(i));
    addOutputPort(oname(i));
  }

  addOutputPort("shift_out");

  addInputPorts(QStringList() << "s" << "e");
}

bool
CQRShiftGate::
exec()
{
  bool s = inputs_[ 9]->getValue();
  bool e = inputs_[10]->getValue();

  bool ov[8];

  ov[7] = inputs_[0]->getValue(); // shift_in

  bool so = inputs_[1]->getValue();

  for (int i = 0; i < 7; ++i)
    ov[i] = inputs_[i + 2]->getValue();

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
      outputs_[8]->setValue(iv);

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

  renderer->painter->strokePath(path, renderer->painter->pen());

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

  double dx = std::abs(p3.x() - p2.x());
  double dy = std::abs(p3.y() - p2.y());

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
    else
      addLine(p1, p4);
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
    else
      addLine(p1, p4);
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

  renderer->painter->drawLine(p1, p2);

  if (showText && renderer->schem->isShowConnectionText())
    CQSchem::drawTextAtPoint(renderer, QPointF((p1.x() + p2.x())/2, (p1.y() + p2.y())/2), name());

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
    return Qt::cyan;

  if (getValue())
    return Qt::green;

  return (isSelected() ? Qt::yellow : Qt::white);
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
  assert(i >= 0 && i < n_);

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
  double x1 = 0.0, y1 = 0.0;
  double x2 = 0.0, y2 = 0.0;
  double yc = 0.0, xc = 0.0;

  for (int i = 0; i < n_; ++i) {
    QPointF p = connections_[i]->midPoint();

    x1 = (i == 0 ? p.x() : std::min(x1, p.x()));
    y1 = (i == 0 ? p.y() : std::min(y1, p.y()));
    x2 = (i == 0 ? p.x() : std::max(x2, p.x()));
    y2 = (i == 0 ? p.y() : std::max(y2, p.y()));

    if      (position() == CQBus::Position::START) {
      xc = (i == 0 ? p.x() : std::min(xc, p.x()));
      yc = (i == 0 ? p.y() : std::min(yc, p.y()));
    }
    else if (position() == CQBus::Position::END) {
      xc = (i == 0 ? p.x() : std::max(xc, p.x()));
      yc = (i == 0 ? p.y() : std::max(yc, p.y()));
    }
    else {
      xc += p.x();
      yc += p.y();
    }
  }

  if (position() == CQBus::Position::MIDDLE) {
    xc /= n_;
    yc /= n_;
  }

  //---

  if      (input) {
    CQPort::Side side = connections_[0]->outPorts()[0]->side();

    bool lr = (side == CQPort::Side::LEFT || side == CQPort::Side::RIGHT);

    double dx = (lr ? mapWidth (0.25) : mapWidth (0.03));
    double dy = (lr ? mapHeight(0.03) : mapHeight(0.25));

    QPointF p1;

    if (lr) {
      if (side == CQPort::Side::LEFT)
        p1 = QPointF(x1 - dx, yc - n_*dy/2.0);
      else
        p1 = QPointF(x2 + dx, yc + n_*dy/2.0);
    }
    else {
      if (side == CQPort::Side::TOP)
        p1 = QPointF(xc + n_*dx/2.0, y1 - dy);
      else
        p1 = QPointF(xc - n_*dx/2.0, y2 + dy);
    }

    // count number above, below
    int na = 0, nb = 0;

    for (int i = 0; i < n_; ++i) {
      QPointF p2 = connections_[i]->midPoint();

      if (lr) {
        if (p2.y() < yc) ++na;
        else             ++nb;
      }
      else {
        if (p2.x() < xc) ++na;
        else             ++nb;
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

        if (p2.y() < yc) {
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

        if (p2.x() < xc) {
          pm1 = QPointF(p1.x(), ym - ia*dx/2.0);

          ++ia;
        }
        else {
          pm1 = QPointF(p1.x(), ym - (nb - ib - 1)*dx/2.0);

          ++ib;
        }

        pm2 = QPointF(p2.x(), pm1.y());
      }

      renderer->painter->drawLine(p1 , pm1);
      renderer->painter->drawLine(pm1, pm2);
      renderer->painter->drawLine(pm2, p2 );

      for (const auto &port : connections_[i]->outPorts())
        CQSchem::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText())
        CQSchem::drawTextAtPoint(renderer,
          QPointF((p1.x() + pm1.x())/2, (p1.y() + pm1.y())/2), connections_[i]->name());

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

    bool lr = (side == CQPort::Side::LEFT || side == CQPort::Side::RIGHT);

    double dx = (lr ? mapWidth (0.25) : mapWidth (0.03));
    double dy = (lr ? mapHeight(0.03) : mapHeight(0.25));

    QPointF p1;

    if (lr) {
      if (side == CQPort::Side::LEFT)
        p1 = QPointF(x1 - dx, yc + n_*dy/2.0);
      else
        p1 = QPointF(x2 + dx, yc - n_*dy/2.0);
    }
    else {
      if (side == CQPort::Side::TOP)
        p1 = QPointF(xc - n_*dx/2.0, y1 - dy);
      else
        p1 = QPointF(xc + n_*dx/2.0, y2 + dy);
    }

    // count number above, below
    int na = 0, nb = 0;

    for (int i = 0; i < n_; ++i) {
      QPointF p2 = connections_[i]->midPoint();

      if (lr) {
        if (p2.y() < yc) ++na;
        else             ++nb;
      }
      else {
        if (p2.x() < xc) ++na;
        else             ++nb;
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

        if (p2.y() < yc) {
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

        if (p2.x() < xc) {
          pm1 = QPointF(p1.x(), ym + ia*dx/2.0);

          ++ia;
        }
        else {
          pm1 = QPointF(p1.x(), ym + (nb - ib - 1)*dx/2.0);

          ++ib;
        }

        pm2 = QPointF(p2.x(), pm1.y());
      }

      renderer->painter->drawLine(p1 , pm1);
      renderer->painter->drawLine(pm1, pm2);
      renderer->painter->drawLine(pm2, p2 );

      for (const auto &port : connections_[i]->inPorts())
        CQSchem::drawConnection(renderer, p2, port->pixelPos());

      if (renderer->schem->isShowConnectionText())
        CQSchem::drawTextAtPoint(renderer,
          QPointF((p1.x() + pm1.x())/2, (p1.y() + pm1.y())/2), connections_[i]->name());

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

  for (auto &gateData : gates_) {
    if (gateData.gate == gate)
      break;

    ++i;
  }

  for ( ; i < n - 1; ++i)
    gates_[i] = gates_[i + 1];

  gates_.pop_back();

  rectValid_ = false;
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

  placementGroup->parentPlacementGroup = this;

  rectValid_ = false;
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
    return Qt::cyan;

  return (isSelected() ? Qt::yellow : QColor(150,150,250));
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

  prect_ = renderer->windowToPixel(rect());

  renderer->painter->drawRect(prect_);

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->draw(renderer);
  }
}
