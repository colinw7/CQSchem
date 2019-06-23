#include <CQSchem.h>
#include <QApplication>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <cassert>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQSchemWindow *window = new CQSchemWindow;

  CQSchem *schem = window->schem();

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg = &argv[i][1];

      if      (arg == "nand"     ) schem->addNandGate();
      else if (arg == "not"      ) schem->addNotGate();
      else if (arg == "and"      ) schem->addAndGate();
      else if (arg == "and3"     ) schem->addAnd3Gate();
      else if (arg == "and4"     ) schem->addAnd4Gate();
      else if (arg == "memory"   ) schem->addMemoryGate();
      else if (arg == "memory8"  ) schem->addMemory8Gate();
      else if (arg == "enabler"  ) schem->addEnablerGate();
      else if (arg == "register" ) schem->addRegisterGate();
      else if (arg == "decoder4" ) schem->addDecoder4Gate();
      else if (arg == "decoder8" ) schem->addDecoder8Gate();
      else if (arg == "decoder16") schem->addDecoder16Gate();

      else if (arg == "build_not"      ) schem->buildNotGate();
      else if (arg == "build_and"      ) schem->buildAndGate();
      else if (arg == "build_and3"     ) schem->buildAnd3Gate();
      else if (arg == "build_and4"     ) schem->buildAnd4Gate();
      else if (arg == "build_memory"   ) schem->buildMemoryGate();
      else if (arg == "build_memory8"  ) schem->buildMemory8Gate();
      else if (arg == "build_enabler"  ) schem->buildEnablerGate();
      else if (arg == "build_register" ) schem->buildRegisterGate();
      else if (arg == "build_decoder4" ) schem->buildDecoder4Gate();
      else if (arg == "build_decoder8" ) schem->buildDecoder8Gate();
      else if (arg == "build_decoder16") schem->buildDecoder16Gate();
      else if (arg == "build_ram256"   ) schem->buildRam256();

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

  schem_ = new CQSchem;

  schem_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  layout->addWidget(schem_);

  //--

  auto addToolButton = [&](const QString &name, const QString &text, bool checked,
                           const char *slotName) {
    QToolButton *button = new QToolButton;

    button->setObjectName(name);
    button->setText(text);
    button->setAutoRaise(true);
    button->setCheckable(true);
    button->setChecked(checked);

    connect(button, SIGNAL(clicked(bool)), this, slotName);

    return button;
  };

  QToolButton *connectionTextButton =
    addToolButton("connectionText", "Connection Text", schem_->isShowConnectionText(),
                  SLOT(connectionTextSlot(bool)));
  QToolButton *gateTextButton =
    addToolButton("gateText"      , "Gate Text"      , schem_->isShowGateText(),
                  SLOT(gateTextSlot(bool)));

  controlLayout->addWidget(connectionTextButton);
  controlLayout->addWidget(gateTextButton);
  controlLayout->addStretch(1);
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

QSize
CQSchemWindow::
sizeHint() const
{
  return QSize(800, 800);
}

//------

CQSchem::
CQSchem()
{
  setFocusPolicy(Qt::StrongFocus);

  //---

#if 0
  QFont font = this->font();

  double s = font.pointSizeF();

  font.setPointSizeF(s*1.5);

  setFont(font);
#endif
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

  CQBus *ibus = new CQBus(8);
  CQBus *obus = new CQBus(8);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

    gate->connect(iname, coni);
    gate->connect(oname, cono);

    ibus->addConnection(coni, i);
    obus->addConnection(cono, i);
  }
}

void
CQSchem::
addEnablerGate()
{
  CQEnablerGate *gate = addGateT<CQEnablerGate>("E");

  CQConnection *cone = addConnection("e");

  gate->connect("e", cone);

  CQBus *ibus = new CQBus(8);
  CQBus *obus = new CQBus(8);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQEnablerGate::iname(i);
    QString oname = CQEnablerGate::oname(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

    gate->connect(iname, coni);
    gate->connect(oname, cono);

    ibus->addConnection(coni, i);
    obus->addConnection(cono, i);
  }
}

void
CQSchem::
addRegisterGate()
{
  CQRegisterGate *gate = addGateT<CQRegisterGate>("R");

  CQConnection *cons = addConnection("s");
  CQConnection *cone = addConnection("e");

  gate->connect("s", cons);
  gate->connect("e", cone);

  CQBus *ibus = new CQBus(8);
  CQBus *obus = new CQBus(8);

  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

    gate->connect(iname, coni);
    gate->connect(oname, cono);

    ibus->addConnection(coni, i);
    obus->addConnection(cono, i);
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
  CQConnection *out1 = addConnection("c");

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
}

void
CQSchem::
buildAnd4Gate()
{
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

  placementGroup->addGate(nandGate1, 0, 0);
  placementGroup->addGate(nandGate2, 1, 1);
  placementGroup->addGate(nandGate3, 0, 2);
  placementGroup->addGate(nandGate4, 1, 2);
}

void
CQSchem::
buildMemory8Gate()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQConnection *cons = addConnection("s");

  for (int i = 0; i < 8; ++i) {
    QString memname = QString("mem%1").arg(i);

    CQMemoryGate *mem = addGateT<CQMemoryGate>(memname);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

    mem->connect("i", coni);
    mem->connect("o", cono);

    mem->connect("s", cons);

    placementGroup->addGate(mem);
  }
}

void
CQSchem::
buildEnablerGate()
{
  CQPlacementGroup *placementGroup = addPlacementGroup(CQPlacementGroup::Placement::VERTICAL);

  //---

  CQConnection *cone = addConnection("e");

  for (int i = 0; i < 8; ++i) {
    QString andname = QString("and%1").arg(i);

    CQAndGate *gate = addGateT<CQAndGate>(andname);

    QString iname = QString("i%1").arg(i);
    QString oname = QString("o%1").arg(i);

    CQConnection *coni = addConnection(iname);
    CQConnection *cono = addConnection(oname);

    gate->connect("a", coni);
    gate->connect("b", cone);
    gate->connect("c", cono);

    placementGroup->addGate(gate);
  }
}

void
CQSchem::
buildRegisterGate()
{
  CQMemory8Gate *mem = addGateT<CQMemory8Gate>("B");
  CQEnablerGate *ena = addGateT<CQEnablerGate>("E");

  CQConnection *cons = addConnection("s");
  CQConnection *cone = addConnection("e");

  mem->connect("s", cons);
  ena->connect("e", cone);

  CQBus *ibus  = new CQBus(8);
  CQBus *iobus = new CQBus(8);
  CQBus *obus  = new CQBus(8);

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
    placementGroup->addGate(notgate[i], i, 1 - i);

  for (int i = 0; i < 4; ++i)
    placementGroup->addGate(andgate[i], i + 1, 2);
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
    placementGroup->addGate(notgate[i], i, 2 - i);

  for (int i = 0; i < 8; ++i)
    placementGroup->addGate(andgate[i], i + 2, 3);
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
    placementGroup->addGate(notgate[i], i, 3 - i);

  for (int i = 0; i < 16; ++i)
    placementGroup->addGate(andgate[i], i + 3, 4);
}

void
CQSchem::
buildRam256()
{
  CQPlacementGroup *placementGroup =
    addPlacementGroup(CQPlacementGroup::Placement::GRID, 2, 2);

  //---

  CQRegisterGate *rgate = addGateT<CQRegisterGate>("R");

  placementGroup->addGate(rgate, 0, 0);

  //--

  CQDecoder16Gate *hdec = addGateT<CQDecoder16Gate>("4x16");
  CQDecoder16Gate *vdec = addGateT<CQDecoder16Gate>("4x16");

  placementGroup->addGate(hdec, 0, 1, 1, 1, CQPlacementGroup::Alignment::FILL);
  placementGroup->addGate(vdec, 1, 0, 1, 1, CQPlacementGroup::Alignment::FILL);

  hdec->setDirection(CQGate::Direction::HORIZONTAL);

  //---

  CQBus *ibus  = new CQBus(8);
  CQBus *obus1 = new CQBus(4);
  CQBus *obus2 = new CQBus(4);

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

  CQBus *iobus = new CQBus(8);

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

      CQAndGate *xgate = addGateT<CQAndGate>("X");

      xgate->connect("a", hout[r]);
      xgate->connect("b", vout[c]);

      CQConnection *t1 = addConnection("t1");

      xgate->connect("c", t1);

      CQAndGate *agate = addGateT<CQAndGate>();
      CQAndGate *bgate = addGateT<CQAndGate>();

      CQRegisterGate *rgate = addGateT<CQRegisterGate>("R");

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

      placementGroup1->addPlacementGroup(placementGroup2, r, c);
    }
  }

  placementGroup->addPlacementGroup(placementGroup1, 1, 1);
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

  placementGroup_.addGate(gate);
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
  placementGroup_.addPlacementGroup(placementGroup);
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
  placementGroup_.place();

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

  renderer_.displayRange.setPixelRange(0, this->height() - 1, this->width() - 1, 0);

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

  for (const auto &connection : connections_)
    connection->draw(&renderer_);

  placementGroup_.draw(&renderer_);
}

void
CQSchem::
mousePressEvent(QMouseEvent *e)
{
  pressed_    = false;
  pressPoint_ = QPointF(e->x(), e->y());

  if      (e->button() == Qt::LeftButton) {
    pressGate_ = nullptr;

    CQGate *gate = nullptr;

    for (auto &gate1 : gates_) {
      if (gate1->inside(pressPoint_)) {
        gate = gate1;
        break;
      }
    }

    if (gate) {
      pressGate_ = gate;

      gate->setSelected(! gate->isSelected());

      update();
    }

    pressed_ = true;
  }
  else if (e->button() == Qt::MidButton) {
    CQConnection *iconnection = nullptr;
    CQConnection *oconnection = nullptr;

    for (auto &gate : gates_) {
      for (auto &port : gate->inputs()) {
        if (port->connection() && port->connection()->inside(pressPoint_)) {
          iconnection = port->connection();
          break;
        }
      }

      for (auto &port : gate->outputs()) {
        if (port->connection() && port->connection()->inside(pressPoint_)) {
          oconnection = port->connection();
          break;
        }
      }
    }

    if (iconnection || oconnection) {
      if (iconnection) {
        iconnection->setValue(! iconnection->getValue());

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
  if (! pressed_)
    return;

  movePoint_ = QPointF(e->x(), e->y());

  if (pressGate_) {
    QPointF p1 = renderer_.pixelToWindow(QPointF(pressPoint_.x(), pressPoint_.y()));
    QPointF p2 = renderer_.pixelToWindow(QPointF(movePoint_ .x(), movePoint_ .y()));

    pressGate_->setRect(pressGate_->rect().translated(p2.x() - p1.x(), p2.y() - p1.y()));

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

  update();
}

void
CQSchem::
drawTextInRect(CQSchemRenderer *renderer, const QRectF &r, const QString &text)
{
  renderer->setFontSize(r.height());

  QPointF c = r.center();

  drawTextAtPoint(renderer, c, text);
}

void
CQSchem::
drawTextAtPoint(CQSchemRenderer *renderer, const QPointF &p, const QString &text)
{
  renderer->setFontSize(renderer->windowHeightToPixelHeight(0.25));

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
  return QSizeF(1.0, 1.0);
}

void
CQGate::
draw(CQSchemRenderer *renderer) const
{
  if (renderer->schem->isShowGateText())
    CQSchem::drawTextInRect(renderer, prect_, name());

//renderer->painter->setPen(Qt::red);
//renderer->painter->drawRect(prect_);
}

void
CQGate::
drawAnd(CQSchemRenderer *renderer, double x1, double y1, double x2, double y2) const
{
  double xm = (x1 + x2)/2.0;
  double ym = (y1 + y2)/2.0;

  QPainterPath path;

  path.moveTo(x1, y1);
  path.lineTo(xm, y1);
  path.quadTo(x2, y1, x2, ym);
  path.quadTo(x2, y2, xm, y2);
  path.lineTo(x1, y2);

  path.closeSubpath();

  renderer->painter->strokePath(path, renderer->painter->pen());
}

void
CQGate::
drawNotIndicator(CQSchemRenderer *renderer, double x, double y) const
{
  double ew = renderer->windowWidthToPixelWidth  (0.05);
  double eh = renderer->windowHeightToPixelHeight(0.05);

  renderer->painter->drawEllipse(QRectF(x - ew/2, y - eh/2, ew, eh));
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

//---

CQNandGate::
CQNandGate(const QString &name) :
 CQGate(name)
{
  addInputPort ("a");
  addInputPort ("b");
  addOutputPort("c");
}

bool
CQNandGate::
exec()
{
  bool b = ! (inputs_[0]->getValue() && inputs_[1]->getValue());

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQNandGate::
draw(CQSchemRenderer *renderer) const
{
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
//double x1 = mapX(0.0);
  double x2 = mapX(0.2);
//double x3 = mapX(0.5);
  double x4 = mapX(0.8);
//double x5 = mapX(1.0);

  double y1 = mapY(0.1);
  double y2 = mapY(0.3);
  double y3 = mapY(0.7);
  double y4 = mapY(0.9);

  double ym = (y1 + y4)/2;

  prect_ = QRectF(x2, y1, x4 - x2, y4 - y1);

  //---

  // draw gate
  drawAnd(renderer, x2, y1, x4, y4);

  drawNotIndicator(renderer, x4, ym);

  //---

  // draw a, b and c connections
  CQPort *a = getPortByName("a");
  CQPort *b = getPortByName("b");
  CQPort *c = getPortByName("c");

  a->setPixelPos(QPointF(x2, y2));
  b->setPixelPos(QPointF(x2, y3));
  c->setPixelPos(QPointF(x4, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQNotGate::
CQNotGate(const QString &name) :
 CQGate(name)
{
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
//double x1 = mapX(0.0);
  double x2 = mapX(0.3);
  double x3 = mapX(0.6);
//double x4 = mapX(0.9);

  double y1 = mapY(0.3);
  double y2 = mapY(0.7);

  double ym = (y1 + y2)/2;

  prect_ = QRectF(x2, y1, x3 - x2, y2 - y1);

  //---

  // draw gate
  QPainterPath path;

  path.moveTo(x2, y1);
  path.lineTo(x2, y2);
  path.lineTo(x3, ym);

  path.closeSubpath();

  renderer->painter->strokePath(path, renderer->painter->pen());

  drawNotIndicator(renderer, x3, ym);

  //---

  // draw a and c connections
  CQPort *a = getPortByName("a");
  CQPort *c = getPortByName("c");

  a->setPixelPos(QPointF(x2, ym));
  c->setPixelPos(QPointF(x3, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQAndGate::
CQAndGate(const QString &name) :
 CQGate(name)
{
  addInputPort ("a");
  addInputPort ("b");
  addOutputPort("c");
}

bool
CQAndGate::
exec()
{
  bool b = inputs_[0]->getValue() && inputs_[1]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAndGate::
draw(CQSchemRenderer *renderer) const
{
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
//double x1 = mapX(0.0);
  double x2 = mapX(0.2);
//double x3 = mapX(0.5);
  double x4 = mapX(0.8);
//double x5 = mapX(1.0);

  double y1 = mapY(0.1);
  double y2 = mapY(0.3);
  double y3 = mapY(0.7);
  double y4 = mapY(0.9);

  double ym = (y1 + y4)/2;

  prect_ = QRectF(x2, y1, x4 - x2, y4 - y1);

  //---

  // draw gate
  drawAnd(renderer, x2, y1, x4, y4);

  //---

  // draw a, b and c connections
  CQPort *a = getPortByName("a");
  CQPort *b = getPortByName("b");
  CQPort *c = getPortByName("c");

  a->setPixelPos(QPointF(x2, y2));
  b->setPixelPos(QPointF(x2, y3));
  c->setPixelPos(QPointF(x4, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQAnd3Gate::
CQAnd3Gate(const QString &name) :
 CQGate(name)
{
  addInputPort ("a");
  addInputPort ("b");
  addInputPort ("c");
  addOutputPort("d");
}

bool
CQAnd3Gate::
exec()
{
  bool b = inputs_[0]->getValue() && inputs_[1]->getValue() && inputs_[2]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAnd3Gate::
draw(CQSchemRenderer *renderer) const
{
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
//double x1 = mapX(0.0);
  double x2 = mapX(0.2);
//double x3 = mapX(0.5);
  double x4 = mapX(0.8);
//double x5 = mapX(1.0);

  double y1 = mapY(0.1);
  double y2 = mapY(0.3);
  double y3 = mapY(0.5);
  double y4 = mapY(0.7);
  double y5 = mapY(0.9);

  double ym = (y1 + y5)/2;

  prect_ = QRectF(x2, y1, x4 - x2, y5 - y1);

  //---

  // draw gate
  drawAnd(renderer, x2, y1, x4, y5);

  //---

  // draw a, b and c connections
  CQPort *a = getPortByName("a");
  CQPort *b = getPortByName("b");
  CQPort *c = getPortByName("c");
  CQPort *d = getPortByName("d");

  a->setPixelPos(QPointF(x2, y2));
  b->setPixelPos(QPointF(x2, y3));
  c->setPixelPos(QPointF(x2, y4));
  d->setPixelPos(QPointF(x4, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQAnd4Gate::
CQAnd4Gate(const QString &name) :
 CQGate(name)
{
  addInputPort ("a");
  addInputPort ("b");
  addInputPort ("c");
  addInputPort ("d");
  addOutputPort("e");
}

bool
CQAnd4Gate::
exec()
{
  bool b = inputs_[0]->getValue() && inputs_[1]->getValue() &&
           inputs_[2]->getValue() && inputs_[3]->getValue();

  if (b == outputs_[0]->getValue())
    return false;

  outputs_[0]->setValue(b);

  return true;
}

void
CQAnd4Gate::
draw(CQSchemRenderer *renderer) const
{
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
//double x1 = mapX(0.0);
  double x2 = mapX(0.2);
//double x3 = mapX(0.5);
  double x4 = mapX(0.8);
//double x5 = mapX(1.0);

  double ygap = (1.0 - 2*margin_)/5.0;

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);

  double y[4];

  for (int i = 0; i < 4; ++i)
    y[i] = mapY(margin_ + (i + 1)*ygap);

  double ym = (y1 + y2)/2;

  prect_ = QRectF(x2, y1, x4 - x2, y2 - y1);

  //---

  // draw gate
  drawAnd(renderer, x2, y1, x4, y2);

  //---

  // draw a, b and c connections
  CQPort *a = getPortByName("a");
  CQPort *b = getPortByName("b");
  CQPort *c = getPortByName("c");
  CQPort *d = getPortByName("d");
  CQPort *e = getPortByName("e");

  a->setPixelPos(QPointF(x2, y[0]));
  b->setPixelPos(QPointF(x2, y[1]));
  c->setPixelPos(QPointF(x2, y[2]));
  d->setPixelPos(QPointF(x2, y[3]));
  e->setPixelPos(QPointF(x4, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQMemoryGate::
CQMemoryGate(const QString &name) :
 CQGate(name)
{
  addInputPort ("i");
  addInputPort ("s");
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
  double x1 = mapX(0.1);
  double x2 = mapX(0.9);

  double y1 = mapY(0.1);
  double y2 = mapY(0.3);
  double y3 = mapY(0.7);
  double y4 = mapY(0.9);

  double ym = (y2 + y3)/2.0;

  prect_ = QRectF(x1, y1, x2 - x1, y4 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw i, o and s connections
  CQPort *i = getPortByName("i");
  CQPort *o = getPortByName("s");
  CQPort *s = getPortByName("o");

  i->setPixelPos(QPointF(x1, y2));
  o->setPixelPos(QPointF(x1, y3));
  s->setPixelPos(QPointF(x2, ym));

  //---

  CQGate::draw(renderer);
}

//---

CQMemory8Gate::
CQMemory8Gate(const QString &name) :
 CQGate(name)
{
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
  double ygap = (1.0 - 2*margin_)/8.0;

  double x1 = mapX(3*margin_);
  double x2 = mapX(1.0 - 3*margin_);
  double x3 = x1 + (x2 - x1)/3.0;

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw i, o and s connections
  for (int i = 0; i < 8; ++i) {
    double y = mapY(margin_ + (i + 0.5)*ygap);

    QString iname = CQMemory8Gate::iname(i);
    QString oname = CQMemory8Gate::oname(i);

    CQPort *coni = getPortByName(iname);
    CQPort *cono = getPortByName(oname);

    coni->setPixelPos(QPointF(x1, y));
    cono->setPixelPos(QPointF(x2, y));
  }

  CQPort *cons = getPortByName("s");

  cons->setPixelPos(QPointF(x3, y2));

  cons->setSide(CQPort::Side::BOTTOM);

  //---

  CQGate::draw(renderer);
}

//---

CQEnablerGate::
CQEnablerGate(const QString &name) :
 CQGate(name)
{
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
  double ygap = (1.0 - 2*margin_)/8.0;

  double x1 = mapX(3*margin_);
  double x2 = mapX(1.0 - 3*margin_);
  double x3 = x1 + (x2 - x1)/3.0;

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw i, o and e connections
  for (int i = 0; i < 8; ++i) {
    double y = mapY(margin_ + (i + 0.5)*ygap);

    QString iname = CQEnablerGate::iname(i);
    QString oname = CQEnablerGate::oname(i);

    CQPort *coni = getPortByName(iname);
    CQPort *cono = getPortByName(oname);

    coni->setPixelPos(QPointF(x1, y));
    cono->setPixelPos(QPointF(x2, y));
  }

  CQPort *cone = getPortByName("e");

  cone->setPixelPos(QPointF(x3, y2));

  cone->setSide(CQPort::Side::BOTTOM);

  //---

  CQGate::draw(renderer);
}

//---

CQRegisterGate::
CQRegisterGate(const QString &name) :
 CQGate(name)
{
  for (int i = 0; i < 8; ++i) {
    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    addInputPort (iname);
    addOutputPort(oname);
  }

  addInputPort("s");
  addInputPort("e");
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  // calc coords
  double ygap = (1.0 - 2*margin_)/8.0;

  double x1 = mapX(3*margin_);
  double x2 = mapX(1.0 - 3*margin_);
  double x3 = x1 + 1.0*(x2 - x1)/3.0;
  double x4 = x1 + 2.0*(x2 - x1)/3.0;

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw i, o and e connections
  for (int i = 0; i < 8; ++i) {
    double y = mapY(margin_ + (i + 0.5)*ygap);

    QString iname = CQRegisterGate::iname(i);
    QString oname = CQRegisterGate::oname(i);

    CQPort *coni = getPortByName(iname);
    CQPort *cono = getPortByName(oname);

    coni->setPixelPos(QPointF(x1, y));
    cono->setPixelPos(QPointF(x2, y));
  }

  CQPort *cons = getPortByName("s");
  CQPort *cone = getPortByName("e");

  cons->setPixelPos(QPointF(x3, y2));
  cone->setPixelPos(QPointF(x4, y2));

  cons->setSide(CQPort::Side::BOTTOM);
  cone->setSide(CQPort::Side::BOTTOM);

  //---

  CQGate::draw(renderer);
}

//---

CQDecoder4Gate::
CQDecoder4Gate(const QString &name) :
 CQGate(name)
{
  addInputPort("a");
  addInputPort("b");

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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  auto mapHeight = [&](double y) {
    return mapY(y) - mapY(0);
  };

  // calc coords
  double ygap = (1.0 - 2*margin_)/4.0;

  double x1 = mapX(3*margin_);
  double x2 = mapX(1.0 - 3*margin_);

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);
  double ym = (y1 + y2)/2.0;
  double yt = ym - mapHeight(0.5*margin_);
  double yb = ym + mapHeight(0.5*margin_);

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw a, b and c connections
  CQPort *cona = getPortByName("a");
  CQPort *conb = getPortByName("b");

  cona->setPixelPos(QPointF(x1, yt));
  conb->setPixelPos(QPointF(x1, yb));

  for (int i = 0; i < 4; ++i) {
    QString oname = CQDecoder4Gate::oname(i);

    CQPort *cono = getPortByName(oname);

    double y = mapY(margin_ + (i + 0.5)*ygap);

    cono->setPixelPos(QPointF(x2, y));
  }

  //---

  CQGate::draw(renderer);
}

//---

CQDecoder8Gate::
CQDecoder8Gate(const QString &name) :
 CQGate(name)
{
  addInputPort("a");
  addInputPort("b");
  addInputPort("c");

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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) { return renderer->windowToPixel(QPointF(x + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) { return renderer->windowToPixel(QPointF(0.0, y + rect_.y())).y(); };

  auto mapHeight = [&](double y) {
    return mapY(y) - mapY(0);
  };

  // calc coords
  double ygap = (1.0 - 2*margin_)/8.0;

  double x1 = mapX(3*margin_);
  double x2 = mapX(1.0 - 3*margin_);

  double y1 = mapY(margin_);
  double y2 = mapY(1.0 - margin_);
  double ym = (y1 + y2)/2.0;
  double yt = ym - mapHeight(margin_);
  double yb = ym + mapHeight(margin_);

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  //---

  // draw gate
  renderer->painter->drawRect(prect_);

  //---

  // draw a, b and c connections
  CQPort *cona = getPortByName("a");
  CQPort *conb = getPortByName("b");
  CQPort *conc = getPortByName("c");

  cona->setPixelPos(QPointF(x1, yt));
  conb->setPixelPos(QPointF(x1, ym));
  conc->setPixelPos(QPointF(x1, yb));

  for (int i = 0; i < 8; ++i) {
    QString oname = CQDecoder8Gate::oname(i);

    CQPort *cono = getPortByName(oname);

    double y = mapY(margin_ + (i + 0.5)*ygap);

    cono->setPixelPos(QPointF(x2, y));
  }

  //---

  CQGate::draw(renderer);
}

//---

CQDecoder16Gate::
CQDecoder16Gate(const QString &name) :
 CQGate(name)
{
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
  renderer->painter->setPen(penColor());

  auto mapX = [&](double x) {
    return renderer->windowToPixel(QPointF(x*rect_.width () + rect_.x(), 0.0)).x(); };
  auto mapY = [&](double y) {
    return renderer->windowToPixel(QPointF(0.0, y*rect_.height() + rect_.y())).y(); };

  auto mapWidth  = [&](double w) { return renderer->windowWidthToPixelWidth  (w*rect_.width ()); };
  auto mapHeight = [&](double h) { return renderer->windowHeightToPixelHeight(h*rect_.height()); };

  //---

  double gap = (1.0 - 2*margin_)/16.0;

  //---

  // draw gate
  double x1, y1, x2, y2;

  if (direction() == Direction::VERTICAL) {
    x1 = mapX(3*margin_);
    x2 = mapX(1.0 - 3*margin_);
    y1 = mapY(margin_);
    y2 = mapY(1.0 - margin_);
  }
  else {
    y1 = mapY(3*margin_);
    y2 = mapY(1.0 - 3*margin_);
    x1 = mapX(margin_);
    x2 = mapX(1.0 - margin_);
  }

  prect_ = QRectF(x1, y1, x2 - x1, y2 - y1);

  renderer->painter->drawRect(prect_);

  //---

  // draw input connections (a, b, c and d)
  CQPort *cona = getPortByName("a");
  CQPort *conb = getPortByName("b");
  CQPort *conc = getPortByName("c");
  CQPort *cond = getPortByName("d");

  if (direction() == Direction::VERTICAL) {
    double ym  = (y1 + y2)/2.0;
    double yt1 = ym  - mapHeight(1.5*margin_);
    double yt2 = yt1 + mapHeight(margin_);
    double yb1 = yt2 + mapHeight(margin_);
    double yb2 = yb1 + mapHeight(margin_);

    cona->setPixelPos(QPointF(x1, yt1)); cona->setSide(CQPort::Side::LEFT);
    conb->setPixelPos(QPointF(x1, yt2)); conb->setSide(CQPort::Side::LEFT);
    conc->setPixelPos(QPointF(x1, yb1)); conc->setSide(CQPort::Side::LEFT);
    cond->setPixelPos(QPointF(x1, yb2)); cond->setSide(CQPort::Side::LEFT);
  }
  else {
    double xm  = (x1 + x2)/2.0;
    double xt1 = xm  - mapWidth(1.5*margin_);
    double xt2 = xt1 + mapWidth(margin_);
    double xb1 = xt2 + mapWidth(margin_);
    double xb2 = xb1 + mapWidth(margin_);

    cona->setPixelPos(QPointF(xt1, y1)); cona->setSide(CQPort::Side::BOTTOM);
    conb->setPixelPos(QPointF(xt2, y1)); conb->setSide(CQPort::Side::BOTTOM);
    conc->setPixelPos(QPointF(xb1, y1)); conc->setSide(CQPort::Side::BOTTOM);
    cond->setPixelPos(QPointF(xb2, y1)); cond->setSide(CQPort::Side::BOTTOM);
  }

  // draw output connections
  for (int i = 0; i < 16; ++i) {
    QString oname = CQDecoder16Gate::oname(i);

    CQPort *cono = getPortByName(oname);

    if (direction() == Direction::VERTICAL) {
      double y = mapY(margin_ + (i + 0.5)*gap);

      cono->setPixelPos(QPointF(x2, y)); cono->setSide(CQPort::Side::LEFT);
    }
    else {
      double x = mapX(margin_ + (i + 0.5)*gap);

      cono->setPixelPos(QPointF(x, y2)); cono->setSide(CQPort::Side::BOTTOM);
    }
  }

  //---

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

void
CQConnection::
setValue(bool b)
{
  value_ = b;

  // propagate to output
  for (auto &oport : outPorts())
    oport->setValue(getValue(), false);
}

void
CQConnection::
draw(CQSchemRenderer *renderer) const
{
  static int dl = 64;

  renderer->painter->setPen(penColor());

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

      drawLine(renderer, port->pixelPos(), port->offsetPixelPos());
      drawLine(renderer, port->offsetPixelPos(), p);
      drawLine(renderer, p, QPointF(xo, yo));
    }

    for (const auto &port : outPorts_) {
      QPointF p(port->offsetPixelPos().x(), yo);

      drawLine(renderer, QPointF(xo, yo), p);
      drawLine(renderer, p, port->offsetPixelPos());
      drawLine(renderer, port->offsetPixelPos(), port->pixelPos());
    }
  }
  else if (ni > 1 && no == 0) {
    // calc max x and average y
    double xo = inPorts_[0]->offsetPixelPos().x();
    double yo = 0.0;

    for (const auto &port : inPorts_) {
      xo  = std::max(xo, port->offsetPixelPos().x());
      yo += port->pixelPos().y();
    }

    xo += dl;
    yo /= ni + no;

    //---

    QPointF po(xo, yo);

    for (const auto &port : outPorts_) {
      QPointF pm1((po.x() + port->pixelPos().x())/2.0, port->pixelPos().y());
      QPointF pm2((po.x() + port->pixelPos().x())/2.0, po.y());

      drawLine(renderer, pm1, port->pixelPos());
      drawLine(renderer, pm1, pm2);
      drawLine(renderer, pm2, po);
    }
  }
  else if (no > 1 && ni == 0) {
    double xo = outPorts_[0]->pixelPos().x();
    double yo = 0.0;

    for (const auto &port : outPorts_) {
      xo  = std::min(xo, port->pixelPos().x());
      yo += port->pixelPos().y();
    }

    xo -= dl;
    yo /= ni + no;

    QPointF po(xo, yo);

    for (const auto &port : outPorts_) {
      QPointF pm1((po.x() + port->pixelPos().x())/2.0, port->pixelPos().y());
      QPointF pm2((po.x() + port->pixelPos().x())/2.0, po.y());

      drawLine(renderer, pm1, port->pixelPos());
      drawLine(renderer, pm1, pm2);
      drawLine(renderer, pm2, po);
    }
  }
  else if (ni == 1 && no == 1) {
    p1 = inPorts_ [0]->pixelPos();
    p2 = outPorts_[0]->pixelPos();

    QPointF p3 = inPorts_ [0]->offsetPixelPos();
    QPointF p4 = outPorts_[0]->offsetPixelPos();

//  double xm = (p3.x() + p4.x())/2.0;
    double ym = (p3.y() + p4.y())/2.0;

    QPointF p5(p3.x(), ym);
    QPointF p6(p4.x(), ym);

    drawLine(renderer, p1, p3);
    drawLine(renderer, p3, p5);
    drawLine(renderer, p5, p6);
    drawLine(renderer, p6, p4);
    drawLine(renderer, p4, p2);
  }
  else if (ni == 1) {
    p1 = inPorts_[0]->pixelPos();
    p2 = inPorts_[0]->offsetPixelPos();

    drawLine(renderer, p1, p2);
  }
  else if (no == 1) {
    p1 = outPorts_[0]->offsetPixelPos();
    p2 = outPorts_[0]->pixelPos();

    drawLine(renderer, p1, p2);
  }
}

void
CQConnection::
drawLine(CQSchemRenderer *renderer, const QPointF &p1, const QPointF &p2) const
{
  renderer->painter->setPen(penColor());

  renderer->painter->drawLine(p1, p2);

  if (renderer->schem->isShowConnectionText())
    CQSchem::drawTextAtPoint(renderer, QPointF((p1.x() + p2.x())/2, (p1.y() + p2.y())/2), name());

  if      (p1.y() == p2.y()) {
    double h = 8;

    rect_ = QRectF(p1.x(), p1.y() - h/2, p2.x() - p1.x(), h);
  }
  else if (p1.x() == p2.x()) {
    double w = 8;

    rect_ = QRectF(p1.x() - w/2, p1.y(), w, p2.y() - p1.y());
  }
  else {
    rect_ = QRectF(p1.x(), p1.y(), p2.x(), p1.y());
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
    gates_[i] = gates_[i + i];

  gates_.pop_back();
}

void
CQPlacementGroup::
addPlacementGroup(CQPlacementGroup *placementGroup, int r, int c, int nr, int nc,
                  Alignment alignment)
{
  placementGroups_.push_back(PlacementGroupData(placementGroup, r, c, nr, nc, alignment));

  placementGroup->parentPlacementGroup = this;
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

      if (placementGroupData.alignment == CQPlacementGroup::Alignment::FILL) {
        w2 = w1;
        h2 = h1;
      }

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

      if (gateData.alignment == CQPlacementGroup::Alignment::FILL) {
        w2 = w1;
        h2 = h1;
      }

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
  }

  //---

  setRect(QRectF(0, 0, w_, h_));
}

void
CQPlacementGroup::
draw(CQSchemRenderer *renderer) const
{
  QRectF prect = renderer->windowToPixel(rect_);

  renderer->painter->setPen(Qt::red);
  renderer->painter->drawRect(prect);

  for (auto &placementGroupData : placementGroups_) {
    CQPlacementGroup *placementGroup = placementGroupData.placementGroup;

    placementGroup->draw(renderer);
  }
}
