<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile>
  <compound kind="class">
    <name>QObject</name>
    <filename>classQObject.html</filename>
  </compound>
  <compound kind="class">
    <name>QWidget</name>
    <filename>classQWidget.html</filename>
  </compound>
  <compound kind="namespace">
    <name>qmapcontrol</name>
    <filename>namespaceqmapcontrol.html</filename>
    <class kind="class">qmapcontrol::ArrowPoint</class>
    <class kind="class">qmapcontrol::CirclePoint</class>
    <class kind="class">qmapcontrol::Curve</class>
    <class kind="class">qmapcontrol::EmptyMapAdapter</class>
    <class kind="class">qmapcontrol::FixedImageOverlay</class>
    <class kind="class">qmapcontrol::Geometry</class>
    <class kind="class">qmapcontrol::GeometryLayer</class>
    <class kind="class">qmapcontrol::GoogleMapAdapter</class>
    <class kind="class">qmapcontrol::GPS_Position</class>
    <class kind="class">qmapcontrol::ImagePoint</class>
    <class kind="class">qmapcontrol::InvisiblePoint</class>
    <class kind="class">qmapcontrol::Layer</class>
    <class kind="class">qmapcontrol::LineString</class>
    <class kind="class">qmapcontrol::MapAdapter</class>
    <class kind="class">qmapcontrol::MapControl</class>
    <class kind="class">qmapcontrol::MapLayer</class>
    <class kind="class">qmapcontrol::MapNetwork</class>
    <class kind="class">qmapcontrol::OpenAerialMapAdapter</class>
    <class kind="class">qmapcontrol::OSMMapAdapter</class>
    <class kind="class">qmapcontrol::Point</class>
    <class kind="class">qmapcontrol::TileMapAdapter</class>
    <class kind="class">qmapcontrol::WMSMapAdapter</class>
    <class kind="class">qmapcontrol::YahooMapAdapter</class>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::ArrowPoint</name>
    <filename>classqmapcontrol_1_1ArrowPoint.html</filename>
    <base>qmapcontrol::Point</base>
    <member kind="enumeration">
      <type></type>
      <name>Alignment</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>TopLeft</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726a61f66ddc6702462a94d3e231f02b9017</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>TopRight</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726a7e42a96f07eab63a8c9fa8a0526f34f4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>TopMiddle</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726aaf2dc2d869f46c11d4c97c6649b2087a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>BottomLeft</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726ae61b9b6ea2fa75ca500d5bb1eaf6f6fc</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>BottomRight</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726a1640f649d644701a2f4633e6bd88b20c</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>BottomMiddle</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726a6165fc7e37a746212ab2911513d3781f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>Middle</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acdfaca60ec19c0265bac2692d7982726a673e6efef9aafe98078c5552e99c923c</anchor>
      <arglist></arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>geometryClicked</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a685dcab83356e5cc449475f177bb487d</anchor>
      <arglist>(Geometry *geometry, QPoint point)</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>positionChanged</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a807f9cfb1b9d680ca76cf825cc9cf46a</anchor>
      <arglist>(Geometry *geom)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ArrowPoint</name>
      <anchorfile>classqmapcontrol_1_1ArrowPoint.html</anchorfile>
      <anchor>a841a287c3d99b6e796442168cbc96cc8</anchor>
      <arglist>(qreal x, qreal y, int sideLength, qreal heading, QString name=QString(), Alignment alignment=Middle, QPen *pen=0)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QRectF</type>
      <name>boundingBox</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>acbb256b5f9f888e9cd3bb475108ece24</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>QPointF</type>
      <name>coordinate</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a2fbb44b2ed047287d715484d2fda7299</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a029a8b50c439c719aac173bffe4cfb71</anchor>
      <arglist>(Geometry *geom)</arglist>
    </member>
    <member kind="function">
      <type>qreal</type>
      <name>getHeading</name>
      <anchorfile>classqmapcontrol_1_1ArrowPoint.html</anchorfile>
      <anchor>a5c08ef7caea74bc61c2fee079f45be43</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>isVisible</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a08422ee75ab02691943c1ca87e2bc563</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>qreal</type>
      <name>latitude</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a6311aabecac471455760aae4790cff91</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>qreal</type>
      <name>longitude</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a2b0f7ec9068af09bcf151af61a785845</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>name</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a2b0a198f837184bf6fff555cee3ce770</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Geometry *</type>
      <name>parentGeometry</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a771cc513dc079219d5da2c4b81019d7c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QPen *</type>
      <name>pen</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>aed7be2fcd2c1d7bccb55f5ac73d7a662</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QPixmap</type>
      <name>pixmap</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>ae781b15ef7d46695b2a7d2855b3f670f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setBaselevel</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a91f1496833bfda9f7a7ec5fcb02a1895</anchor>
      <arglist>(int zoomlevel)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setHeading</name>
      <anchorfile>classqmapcontrol_1_1ArrowPoint.html</anchorfile>
      <anchor>a553a63cdc0e822aaf3f324d23b86cec7</anchor>
      <arglist>(qreal heading)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setMaxsize</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>adc2724c4e195727b823ff55c940283de</anchor>
      <arglist>(QSize maxsize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setMinsize</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>ac40b3e44f54fab1330b9309ac7bd84d2</anchor>
      <arglist>(QSize minsize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setName</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a6220fae15759fd0fa7d75e415df34e83</anchor>
      <arglist>(QString name)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>setPen</name>
      <anchorfile>classqmapcontrol_1_1ArrowPoint.html</anchorfile>
      <anchor>aa92f0f1b5d2fd424196a33012ffe8ea1</anchor>
      <arglist>(QPen *pen)</arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a3a013a6edb6d10a71297978bc31a796b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>QWidget *</type>
      <name>widget</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>ad1eaabeb2b227cd055ccf4b4e2818480</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual bool</type>
      <name>Touches</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a7dee2100a2d2056511aca25c9390d253</anchor>
      <arglist>(Point *click, const MapAdapter *mapadapter)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::CirclePoint</name>
    <filename>classqmapcontrol_1_1CirclePoint.html</filename>
    <base>qmapcontrol::Point</base>
    <member kind="function">
      <type></type>
      <name>CirclePoint</name>
      <anchorfile>classqmapcontrol_1_1CirclePoint.html</anchorfile>
      <anchor>aa0dd3496708e507c8185d5ae5f5e79ad</anchor>
      <arglist>(qreal x, qreal y, QString name=QString(), Alignment alignment=Middle, QPen *pen=0)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>CirclePoint</name>
      <anchorfile>classqmapcontrol_1_1CirclePoint.html</anchorfile>
      <anchor>a13300765d52da11cc8cbb4384e8e9e23</anchor>
      <arglist>(qreal x, qreal y, int radius=10, QString name=QString(), Alignment alignment=Middle, QPen *pen=0)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>setPen</name>
      <anchorfile>classqmapcontrol_1_1CirclePoint.html</anchorfile>
      <anchor>aa92f0f1b5d2fd424196a33012ffe8ea1</anchor>
      <arglist>(QPen *pen)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::Curve</name>
    <filename>classqmapcontrol_1_1Curve.html</filename>
    <base>qmapcontrol::Geometry</base>
    <member kind="slot" virtualness="virtual">
      <type>virtual void</type>
      <name>setVisible</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>a18e44e30b31525a243960ca3928125aa</anchor>
      <arglist>(bool visible)</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual QRectF</type>
      <name>boundingBox</name>
      <anchorfile>classqmapcontrol_1_1Geometry.html</anchorfile>
      <anchor>af92c4fa46f711bea92efe5ab80f9084d</anchor>
      <arglist>()=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::EmptyMapAdapter</name>
    <filename>classqmapcontrol_1_1EmptyMapAdapter.html</filename>
    <base>qmapcontrol::MapAdapter</base>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>changeHostAddress</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a7807022d8586d6d050541d2344eb12cb</anchor>
      <arglist>(const QString qHost, const QString qServerPath=QString())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QPoint</type>
      <name>coordinateToDisplay</name>
      <anchorfile>classqmapcontrol_1_1EmptyMapAdapter.html</anchorfile>
      <anchor>a94134b06e350d302f5b3a63f0016aa60</anchor>
      <arglist>(const QPointF &amp;) const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>currentZoom</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a8c7b803b9faa35db237e40c361e1c036</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QPointF</type>
      <name>displayToCoordinate</name>
      <anchorfile>classqmapcontrol_1_1EmptyMapAdapter.html</anchorfile>
      <anchor>a601b4631d9d891eabffb063c510cc088</anchor>
      <arglist>(const QPoint &amp;) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>EmptyMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1EmptyMapAdapter.html</anchorfile>
      <anchor>a9208e1a9da209564f85a50318cda7310</anchor>
      <arglist>(int tileSize=256, int minZoom=0, int maxZoom=17)</arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>host</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a7ee8388c7880d8a3466967464f5034b6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>maxZoom</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a3a33e897bc474405772d17a7c81f8747</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>minZoom</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a7457f51db57914a85bbefcc8c9fa97b4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QString</type>
      <name>serverPath</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a12436fe3d5d3cac5930f5b50fb508d36</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>tilesize</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>af105c5a0cf588a3f60d67744bd353391</anchor>
      <arglist>() const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::FixedImageOverlay</name>
    <filename>classqmapcontrol_1_1FixedImageOverlay.html</filename>
    <base>qmapcontrol::ImagePoint</base>
    <member kind="function">
      <type></type>
      <name>FixedImageOverlay</name>
      <anchorfile>classqmapcontrol_1_1FixedImageOverlay.html</anchorfile>
      <anchor>a621806ec022f1b35a2383b64787a5827</anchor>
      <arglist>(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QString filename, QString name=QString())</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FixedImageOverlay</name>
      <anchorfile>classqmapcontrol_1_1FixedImageOverlay.html</anchorfile>
      <anchor>aa5c420b6584232328fc12d8bc699129c</anchor>
      <arglist>(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QPixmap pixmap, QString name=QString())</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::Geometry</name>
    <filename>classqmapcontrol_1_1Geometry.html</filename>
    <base>QObject</base>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::GeometryLayer</name>
    <filename>classqmapcontrol_1_1GeometryLayer.html</filename>
    <base>qmapcontrol::Layer</base>
    <member kind="enumeration">
      <type></type>
      <name>LayerType</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a56943a0946e5f15e5e58054b8e7a04a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>MapLayer</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a56943a0946e5f15e5e58054b8e7a04a4afe7df421203e4175d260b8dabcbe3002</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>GeometryLayer</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a56943a0946e5f15e5e58054b8e7a04a4a6c04bd58c42df8a7539aba782503fee0</anchor>
      <arglist></arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>setVisible</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a18e44e30b31525a243960ca3928125aa</anchor>
      <arglist>(bool visible)</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>geometryClicked</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a685dcab83356e5cc449475f177bb487d</anchor>
      <arglist>(Geometry *geometry, QPoint point)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>addGeometry</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>ab692d7d08414ed2b744946b88872827f</anchor>
      <arglist>(Geometry *geometry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>clearGeometries</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>acb2413f25e560a0cfadb7128d5af99b0</anchor>
      <arglist>(bool qDeleteObject=false)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>containsGeometry</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a0b0b1de1c0e21dbec196d11b336628f4</anchor>
      <arglist>(Geometry *geometry)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>GeometryLayer</name>
      <anchorfile>classqmapcontrol_1_1GeometryLayer.html</anchorfile>
      <anchor>a64e2ab047db14f0d86424bee947c94af</anchor>
      <arglist>(QString layername, MapAdapter *mapadapter, bool takeevents=true)</arglist>
    </member>
    <member kind="function">
      <type>QList&lt; Geometry * &gt; &amp;</type>
      <name>getGeometries</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>ae5f0dce6ce743e714b314f95a6305908</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>isVisible</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a08422ee75ab02691943c1ca87e2bc563</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>layername</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a414e94fdd70490d75ddccb6923ae3410</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Layer::LayerType</type>
      <name>layertype</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a1cfbd8a5c27cf9cb400fa458a1f70ba5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>MapAdapter *</type>
      <name>mapadapter</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a732b5e9de3b67ed69aa7dd165c9778d6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>removeGeometry</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>af2a2f7fec3f6e5fbf623f466a961bfb7</anchor>
      <arglist>(Geometry *geometry, bool qDeleteObject=false)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sendGeometryToBack</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a4a6d2f001e34be8d424ccfb45b8d7622</anchor>
      <arglist>(Geometry *geometry)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sendGeometryToFront</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a42afc531c673c3adc2e38fae58f87d52</anchor>
      <arglist>(Geometry *geometry)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::GoogleMapAdapter</name>
    <filename>classqmapcontrol_1_1GoogleMapAdapter.html</filename>
    <base>qmapcontrol::TileMapAdapter</base>
    <member kind="function" virtualness="virtual">
      <type>virtual QPoint</type>
      <name>coordinateToDisplay</name>
      <anchorfile>classqmapcontrol_1_1TileMapAdapter.html</anchorfile>
      <anchor>a94134b06e350d302f5b3a63f0016aa60</anchor>
      <arglist>(const QPointF &amp;) const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QPointF</type>
      <name>displayToCoordinate</name>
      <anchorfile>classqmapcontrol_1_1TileMapAdapter.html</anchorfile>
      <anchor>a601b4631d9d891eabffb063c510cc088</anchor>
      <arglist>(const QPoint &amp;) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>GoogleMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1GoogleMapAdapter.html</anchorfile>
      <anchor>a95b487b1cc18726c8adcba0450209a31</anchor>
      <arglist>(googleLayerType qLayerType=maps)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::ImagePoint</name>
    <filename>classqmapcontrol_1_1ImagePoint.html</filename>
    <base>qmapcontrol::Point</base>
    <member kind="function">
      <type></type>
      <name>ImagePoint</name>
      <anchorfile>classqmapcontrol_1_1ImagePoint.html</anchorfile>
      <anchor>aa5121dbb37cf1b8924a376e4c7edd728</anchor>
      <arglist>(qreal x, qreal y, QString filename, QString name=QString(), Alignment alignment=Middle)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ImagePoint</name>
      <anchorfile>classqmapcontrol_1_1ImagePoint.html</anchorfile>
      <anchor>a5859a1a7467f65a24ef7009acb927446</anchor>
      <arglist>(qreal x, qreal y, QPixmap pixmap, QString name=QString(), Alignment alignment=Middle)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::InvisiblePoint</name>
    <filename>classqmapcontrol_1_1InvisiblePoint.html</filename>
    <base>qmapcontrol::Point</base>
    <member kind="function">
      <type></type>
      <name>InvisiblePoint</name>
      <anchorfile>classqmapcontrol_1_1InvisiblePoint.html</anchorfile>
      <anchor>a85a9b75f7ffc4fdf3552632ac093bac4</anchor>
      <arglist>(qreal x, qreal y, QString name=QString())</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>InvisiblePoint</name>
      <anchorfile>classqmapcontrol_1_1InvisiblePoint.html</anchorfile>
      <anchor>a87487e17524b00f90d27ac2b097f057a</anchor>
      <arglist>(qreal x, qreal y, int width=10, int height=10, QString name=QString())</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>InvisiblePoint</name>
      <anchorfile>classqmapcontrol_1_1InvisiblePoint.html</anchorfile>
      <anchor>a7285bca61d6639979ee51aae1584c286</anchor>
      <arglist>(qreal x, qreal y, int sideLength=10, QString name=QString())</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::Layer</name>
    <filename>classqmapcontrol_1_1Layer.html</filename>
    <base>QObject</base>
    <member kind="function">
      <type></type>
      <name>Layer</name>
      <anchorfile>classqmapcontrol_1_1Layer.html</anchorfile>
      <anchor>a7ffb001076dc500ad13e523836bda23d</anchor>
      <arglist>(QString layername, MapAdapter *mapadapter, enum LayerType layertype, bool takeevents=true)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::LineString</name>
    <filename>classqmapcontrol_1_1LineString.html</filename>
    <base>qmapcontrol::Curve</base>
    <member kind="function">
      <type>void</type>
      <name>addPoint</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a8694ab9a03b0ed4986c98ad727755f8a</anchor>
      <arglist>(Point *point)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QRectF</type>
      <name>boundingBox</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>acbb256b5f9f888e9cd3bb475108ece24</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QList&lt; Geometry * &gt; &amp;</type>
      <name>clickedPoints</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a3dc2e4e852bf89971b5e2660720745f4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>hasClickedPoints</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a3f7357f0362b6bee75f8c8c623fb528e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>hasPoints</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>ac3fc4ac8c80b5bf64c0bf095d7fde94b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>LineString</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>ad8efdad1cc0ff6c63357cb72180c3a0a</anchor>
      <arglist>(QList&lt; Point * &gt; const points, QString name=QString(), QPen *pen=0)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>numberOfPoints</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a06b5ac0b597b8d1cb7e8817f7e66c2eb</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QList&lt; Point * &gt;</type>
      <name>points</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a18d4d26904bca7c54fb9d2a1b054c2fb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setPoints</name>
      <anchorfile>classqmapcontrol_1_1LineString.html</anchorfile>
      <anchor>a6af8f478f54e6704e87dcf184a258a8c</anchor>
      <arglist>(QList&lt; Point * &gt; points)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::MapAdapter</name>
    <filename>classqmapcontrol_1_1MapAdapter.html</filename>
    <base>QObject</base>
    <member kind="function" virtualness="pure">
      <type>virtual QPoint</type>
      <name>coordinateToDisplay</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>a0a7f30d12395e615eec9440070795349</anchor>
      <arglist>(const QPointF &amp;coordinate) const =0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual QPointF</type>
      <name>displayToCoordinate</name>
      <anchorfile>classqmapcontrol_1_1MapAdapter.html</anchorfile>
      <anchor>aa26c33260233907672b1b23f4b1fd033</anchor>
      <arglist>(const QPoint &amp;point) const =0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::MapControl</name>
    <filename>classqmapcontrol_1_1MapControl.html</filename>
    <base>QWidget</base>
    <member kind="enumeration">
      <type></type>
      <name>MouseMode</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a034ae8169a2202325de4ef39ffd3e431</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>Panning</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a034ae8169a2202325de4ef39ffd3e431ae105bcd8daf776fd01704a7186c49608</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>Dragging</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a034ae8169a2202325de4ef39ffd3e431aea74c0c82481d6d724a43536424e3977</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <type>@</type>
      <name>None</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a034ae8169a2202325de4ef39ffd3e431ac9d3e887722f2bc482bcca9d41c512af</anchor>
      <arglist></arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>resize</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a148f8aec7ea97e2e465cf2bd979846ab</anchor>
      <arglist>(const QSize newSize)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>scroll</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a527394cb8e8aa2d77f7a50a07b9e9f3e</anchor>
      <arglist>(const QPoint scroll)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>scrollDown</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a51db121d79cb0a651a7441b98bb7d7a9</anchor>
      <arglist>(int pixel=10)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>scrollLeft</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a02f3bf431288e7ed34ecc59f7b8de996</anchor>
      <arglist>(int pixel=10)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>scrollRight</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a216e70011cb465e61e2992d761f568df</anchor>
      <arglist>(int pixel=10)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>scrollUp</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>aed27d1373fd9e05fb86fa319df4fe375</anchor>
      <arglist>(int pixel=10)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>setZoom</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>abb4bd8d8137d16816838c97d32407f39</anchor>
      <arglist>(int zoomlevel)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>updateRequest</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a5cb68a198a28000fec8b7de1064d0a41</anchor>
      <arglist>(QRect rect)</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>updateRequestNew</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>acf37bc294477796509e00e8f546fbd44</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>zoomIn</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a7d7e315e34a66d9a66022d31635e7aca</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>zoomOut</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a72d29d38d8dd2c091cdd7078e1364f25</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>boxDragged</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a1fd82da86a16f8932d45f4a0cadb60d1</anchor>
      <arglist>(const QRectF)</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>geometryClicked</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>acc9ddd8e1721682682f85e73dae5f768</anchor>
      <arglist>(Geometry *geometry, QPoint coord_px)</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>mouseEventCoordinate</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>aa463d9dfa43fa385b48aab115d374637</anchor>
      <arglist>(const QMouseEvent *evnt, const QPointF coordinate)</arglist>
    </member>
    <member kind="signal">
      <type>void</type>
      <name>viewChanged</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a2c7862f1cb1b73192e3f4922eaac248d</anchor>
      <arglist>(const QPointF &amp;coordinate, int zoom) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>addLayer</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a87c6ef3c45ee9c21e173db2aa16cf567</anchor>
      <arglist>(Layer *layer)</arglist>
    </member>
    <member kind="function">
      <type>QPointF</type>
      <name>currentCoordinate</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a010c83996ab94a3db104aecf0550d480</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>currentZoom</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a8c7b803b9faa35db237e40c361e1c036</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>enableMouseWheelEvents</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a0020f498f44618941e35c9cd7368dcd7</anchor>
      <arglist>(bool enabled=true)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>enablePersistentCache</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a47398dba0f1da60f1e80bcdd4286765a</anchor>
      <arglist>(int tileExpiry=-1, const QDir &amp;path=QDir::homePath()+&quot;/QMapControl.cache&quot;)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>followGeometry</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a6f7a69381097c74928af4ab3aa970386</anchor>
      <arglist>(const Geometry *geometry) const </arglist>
    </member>
    <member kind="function">
      <type>QRectF</type>
      <name>getBoundingBox</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ab06601b3a0eddcb0bf4d56deb74d8a3d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>QRectF</type>
      <name>getViewport</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ac8185831c1b2df6c0db6b19897be0450</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>isBoundingBoxEnabled</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a74f59e58143713bc1776b111f3bf9baa</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>isGeometryVisible</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a81804b97ecc3e2fff3a64314ff866587</anchor>
      <arglist>(Geometry *geometry)</arglist>
    </member>
    <member kind="function">
      <type>Layer *</type>
      <name>layer</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a8e22ad201cb035fda7d7fefd8f348b11</anchor>
      <arglist>(const QString &amp;layername) const </arglist>
    </member>
    <member kind="function">
      <type>QList&lt; QString &gt;</type>
      <name>layers</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>aee9467ec4674af32aed2fbeb1175e0b5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>loadingQueueSize</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a719be06d19c5beef2f271438e40b2651</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>MapControl</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a7850f3ec2dfcc0f8e436c8253e479222</anchor>
      <arglist>(QWidget *parent=0, Qt::WindowFlags windowFlags=0)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>MapControl</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a1a23dc4a4eb95a63ba65e3b4a79b410d</anchor>
      <arglist>(QSize size, MouseMode mousemode=Panning, bool showScale=false, bool showCrosshairs=true, QWidget *parent=0, Qt::WindowFlags windowFlags=0)</arglist>
    </member>
    <member kind="function">
      <type>MapControl::MouseMode</type>
      <name>mouseMode</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a9106ab9dcac042b57fc4e797c94d84dc</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>mouseWheelEventsEnabled</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a39f5b4d70b45b5e61167f7554598b41d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>moveTo</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>af7e3575f01f98a4096ccf48ac6bd4a50</anchor>
      <arglist>(QPointF coordinate)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>numberOfLayers</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>af3950823abbdf717124c279a386ca280</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>removeLayer</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ae329061c3d6e1a3ae29d047f0ddc68de</anchor>
      <arglist>(Layer *layer)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setBoundingBox</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a852552cdefde98119bd73178df0abd64</anchor>
      <arglist>(QRectF &amp;rect)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setMouseMode</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a346037e336da8525fe41ec30bf216d82</anchor>
      <arglist>(MouseMode mousemode)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setProxy</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a85482ec99b8367726541cfdb51994e98</anchor>
      <arglist>(QString host, int port, const QString username=QString(), const QString password=QString())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setUseBoundingBox</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>adce5194636e5ffd37ddc2ce5cd8e6e5d</anchor>
      <arglist>(bool usebounds)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setView</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ab10ff3e8fed3a535de2a435ab1db48fb</anchor>
      <arglist>(const QPointF &amp;coordinate) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setView</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ac837e5df11959daca31bd1d01d12b94c</anchor>
      <arglist>(const QList&lt; QPointF &gt; coordinates) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setView</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>a4ea85421ec8df905fba209510c909e2c</anchor>
      <arglist>(const Point *point) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setViewAndZoomIn</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ae8c52337a968729d53c1ba57bfd65ea4</anchor>
      <arglist>(const QList&lt; QPointF &gt; coordinates) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>showScale</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>aed8a66d91a8f5fe5e6d02c4e4327eaa8</anchor>
      <arglist>(bool visible)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>stopFollowing</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ab95e8ed0a669d1669000786f880d16a3</anchor>
      <arglist>(const Geometry *geometry) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>updateView</name>
      <anchorfile>classqmapcontrol_1_1MapControl.html</anchorfile>
      <anchor>ab43fe107b5cec09d559630d5a6a789e2</anchor>
      <arglist>() const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::MapLayer</name>
    <filename>classqmapcontrol_1_1MapLayer.html</filename>
    <base>qmapcontrol::Layer</base>
    <member kind="function">
      <type></type>
      <name>MapLayer</name>
      <anchorfile>classqmapcontrol_1_1MapLayer.html</anchorfile>
      <anchor>ac50a4fcff04c6859ce5601543b8ac821</anchor>
      <arglist>(QString layername, MapAdapter *mapadapter, bool takeevents=true)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::MapNetwork</name>
    <filename>classqmapcontrol_1_1MapNetwork.html</filename>
    <base protection="private">QObject</base>
    <member kind="function">
      <type>void</type>
      <name>abortLoading</name>
      <anchorfile>classqmapcontrol_1_1MapNetwork.html</anchorfile>
      <anchor>a882eed9a0b1403a74d494d0a50c0ea61</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>imageIsLoading</name>
      <anchorfile>classqmapcontrol_1_1MapNetwork.html</anchorfile>
      <anchor>a8a715c8ceb27e448bd81606ab33e4bb4</anchor>
      <arglist>(QString url)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>loadQueueSize</name>
      <anchorfile>classqmapcontrol_1_1MapNetwork.html</anchorfile>
      <anchor>ae759808f4d89e825b69945dc72130058</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QNetworkAccessManager *</type>
      <name>nextFreeHttp</name>
      <anchorfile>classqmapcontrol_1_1MapNetwork.html</anchorfile>
      <anchor>a5dae61e2e5fabf9e40d933768101da18</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::OpenAerialMapAdapter</name>
    <filename>classqmapcontrol_1_1OpenAerialMapAdapter.html</filename>
    <base>qmapcontrol::TileMapAdapter</base>
    <member kind="function">
      <type></type>
      <name>OpenAerialMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1OpenAerialMapAdapter.html</anchorfile>
      <anchor>a8b9f8f912aa1bf34e1b4d11e7dfec394</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::OSMMapAdapter</name>
    <filename>classqmapcontrol_1_1OSMMapAdapter.html</filename>
    <base>qmapcontrol::TileMapAdapter</base>
    <member kind="function">
      <type></type>
      <name>OSMMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1OSMMapAdapter.html</anchorfile>
      <anchor>ab6f5dd078b96030385b14d2a5d777092</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::Point</name>
    <filename>classqmapcontrol_1_1Point.html</filename>
    <base>qmapcontrol::Geometry</base>
    <member kind="function">
      <type></type>
      <name>Point</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a47257eee92b14e7c7f9b778c67bcb9a5</anchor>
      <arglist>(qreal x, qreal y, QString name=QString(), enum Alignment alignment=Middle)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Point</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>aa1767675df0bc3c13c75b3a48241125e</anchor>
      <arglist>(qreal x, qreal y, QWidget *widget, QString name=QString(), enum Alignment alignment=Middle)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Point</name>
      <anchorfile>classqmapcontrol_1_1Point.html</anchorfile>
      <anchor>a2a5d75301b8d2fd2ed406bddd0835740</anchor>
      <arglist>(qreal x, qreal y, QPixmap pixmap, QString name=QString(), enum Alignment alignment=Middle)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::TileMapAdapter</name>
    <filename>classqmapcontrol_1_1TileMapAdapter.html</filename>
    <base>qmapcontrol::MapAdapter</base>
    <member kind="function">
      <type></type>
      <name>TileMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1TileMapAdapter.html</anchorfile>
      <anchor>a1233007f4393d3ae476a5284f9294e8c</anchor>
      <arglist>(const QString &amp;host, const QString &amp;serverPath, int tilesize, int minZoom=0, int maxZoom=17)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::WMSMapAdapter</name>
    <filename>classqmapcontrol_1_1WMSMapAdapter.html</filename>
    <base>qmapcontrol::MapAdapter</base>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>changeHostAddress</name>
      <anchorfile>classqmapcontrol_1_1WMSMapAdapter.html</anchorfile>
      <anchor>a7807022d8586d6d050541d2344eb12cb</anchor>
      <arglist>(const QString qHost, const QString qServerPath=QString())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QPoint</type>
      <name>coordinateToDisplay</name>
      <anchorfile>classqmapcontrol_1_1WMSMapAdapter.html</anchorfile>
      <anchor>a94134b06e350d302f5b3a63f0016aa60</anchor>
      <arglist>(const QPointF &amp;) const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QPointF</type>
      <name>displayToCoordinate</name>
      <anchorfile>classqmapcontrol_1_1WMSMapAdapter.html</anchorfile>
      <anchor>a601b4631d9d891eabffb063c510cc088</anchor>
      <arglist>(const QPoint &amp;) const </arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual QString</type>
      <name>serverPath</name>
      <anchorfile>classqmapcontrol_1_1WMSMapAdapter.html</anchorfile>
      <anchor>a12436fe3d5d3cac5930f5b50fb508d36</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>WMSMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1WMSMapAdapter.html</anchorfile>
      <anchor>a0bfce681fddbae34a9fae6fc8c2f0a38</anchor>
      <arglist>(QString host, QString serverPath, int tilesize=256)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>qmapcontrol::YahooMapAdapter</name>
    <filename>classqmapcontrol_1_1YahooMapAdapter.html</filename>
    <base>qmapcontrol::TileMapAdapter</base>
    <member kind="function">
      <type></type>
      <name>YahooMapAdapter</name>
      <anchorfile>classqmapcontrol_1_1YahooMapAdapter.html</anchorfile>
      <anchor>a9755050f27b44221d3e473b1cebd70cf</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="dir">
    <name>src</name>
    <path>D:/Dev/QMapControl/QMapControl/src/</path>
    <filename>dir_68267d1309a1af8e8297ef4c3efbcdba.html</filename>
    <file>arrowpoint.cpp</file>
    <file>arrowpoint.h</file>
    <file>circlepoint.cpp</file>
    <file>circlepoint.h</file>
    <file>curve.cpp</file>
    <file>curve.h</file>
    <file>emptymapadapter.cpp</file>
    <file>emptymapadapter.h</file>
    <file>fixedimageoverlay.cpp</file>
    <file>fixedimageoverlay.h</file>
    <file>geometry.cpp</file>
    <file>geometry.h</file>
    <file>geometrylayer.cpp</file>
    <file>geometrylayer.h</file>
    <file>googlemapadapter.cpp</file>
    <file>googlemapadapter.h</file>
    <file>gps_position.cpp</file>
    <file>gps_position.h</file>
    <file>imagemanager.cpp</file>
    <file>imagepoint.cpp</file>
    <file>imagepoint.h</file>
    <file>invisiblepoint.cpp</file>
    <file>invisiblepoint.h</file>
    <file>layer.cpp</file>
    <file>layer.h</file>
    <file>linestring.cpp</file>
    <file>linestring.h</file>
    <file>mapadapter.cpp</file>
    <file>mapadapter.h</file>
    <file>mapcontrol.cpp</file>
    <file>mapcontrol.h</file>
    <file>maplayer.cpp</file>
    <file>maplayer.h</file>
    <file>mapnetwork.cpp</file>
    <file>mapnetwork.h</file>
    <file>openaerialmapadapter.cpp</file>
    <file>openaerialmapadapter.h</file>
    <file>osmmapadapter.cpp</file>
    <file>osmmapadapter.h</file>
    <file>point.cpp</file>
    <file>point.h</file>
    <file>qmapcontrol_global.h</file>
    <file>tilemapadapter.cpp</file>
    <file>tilemapadapter.h</file>
    <file>wmsmapadapter.cpp</file>
    <file>wmsmapadapter.h</file>
    <file>yahoomapadapter.cpp</file>
    <file>yahoomapadapter.h</file>
  </compound>
</tagfile>
