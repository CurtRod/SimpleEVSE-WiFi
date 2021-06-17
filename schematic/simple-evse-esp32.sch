<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.7.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="101" name="Hidden" color="15" fill="1" visible="yes" active="yes"/>
<layer number="102" name="Changes" color="12" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<description>Simple-EVSE-ESP32</description>
<libraries>
<library name="e-sicherungen">
<description>&lt;b&gt;Sicherungen und Begrenzer für Elektropläne&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Autor librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="FEHLER-STROM-SCHUTZSCHALTER_4-POL">
<description>Fehlerstromschutzschalter 4-pol</description>
<pad name="1" x="0" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="2" x="0" y="-12.7" drill="0.2" diameter="0.6" shape="square"/>
<pad name="3" x="5.08" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="4" x="5.08" y="-12.7" drill="0.2" diameter="0.6" shape="square"/>
<pad name="5" x="10.16" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="6" x="10.16" y="-12.7" drill="0.2" diameter="0.6" shape="square"/>
<pad name="N" x="15.24" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="N'" x="15.24" y="-12.7" drill="0.2" diameter="0.6" shape="square"/>
</package>
<package name="LEITUNGSSCHUTZSCHALTER">
<description>Dummy</description>
<pad name="1" x="2.54" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="2" x="2.54" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<text x="0" y="2.54" size="1.27" layer="25" font="vector">&gt;NAME</text>
<text x="0" y="-2.54" size="1.27" layer="27" font="vector">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="FEHLER-STROM-SCHUTZSCHALTER_4-POL">
<wire x1="-15.4762" y1="0.0138" x2="-13.9512" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-12.93" y1="0.0138" x2="-11.4046" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-10.3836" y1="0.0138" x2="-8.8582" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-7.8372" y1="0.0138" x2="-6.312" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="12.2002" y1="0.0138" x2="9.0108" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="9.3932" y1="1.2902" x2="9.3932" y2="-1.2626" width="0.1524" layer="94"/>
<wire x1="11.9462" y1="1.2902" x2="9.3932" y2="1.2902" width="0.1524" layer="94"/>
<wire x1="11.9462" y1="-1.2626" x2="9.3932" y2="-1.2626" width="0.1524" layer="94"/>
<wire x1="11.9462" y1="1.2902" x2="11.9462" y2="-1.2626" width="0.1524" layer="94"/>
<wire x1="10.6696" y1="1.6682" x2="10.6696" y2="-1.6338" width="0.1524" layer="94"/>
<wire x1="10.6696" y1="-1.6338" x2="10.6696" y2="-1.6404" width="0.1524" layer="94"/>
<wire x1="13.7242" y1="0.0138" x2="13.2148" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-10.16" y1="-2.54" x2="-11.2395" y2="0.762" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-2.54" x2="-6.1595" y2="0.762" width="0.254" layer="94"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-10.668" y1="1.778" x2="-10.16" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-9.652" y1="1.778" x2="-10.16" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="2.54" x2="-5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-5.588" y1="1.778" x2="-5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-4.572" y1="1.778" x2="-5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="13.716" y1="2.54" x2="16.891" y2="2.54" width="0.1524" layer="94"/>
<wire x1="16.891" y1="2.54" x2="16.891" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="16.891" y1="-2.54" x2="13.716" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="13.716" y1="2.54" x2="13.716" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="-2.54" x2="-1.0795" y2="0.762" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-0.508" y1="1.778" x2="0" y2="1.778" width="0.1524" layer="94"/>
<wire x1="0.508" y1="1.778" x2="0" y2="1.778" width="0.1524" layer="94"/>
<wire x1="-5.3036" y1="0.0138" x2="-3.7782" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-2.7572" y1="0.0138" x2="-1.232" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-10.16" y1="-2.54" x2="-10.16" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-2.54" x2="-5.08" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="0" y1="-2.54" x2="0" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-5.0165" x2="6.35" y2="-8.763" width="0.1524" layer="94"/>
<wire x1="8.255" y1="-4.953" x2="9.906" y2="-4.953" width="0.1524" layer="94"/>
<wire x1="11.557" y1="-4.953" x2="12.7" y2="-4.953" width="0.1524" layer="94"/>
<wire x1="12.7" y1="-4.953" x2="12.7" y2="-8.8265" width="0.1524" layer="94"/>
<wire x1="12.7" y1="-8.8265" x2="8.382" y2="-8.8265" width="0.1524" layer="94"/>
<wire x1="9.906" y1="-3.556" x2="9.906" y2="-5.3975" width="0.1524" layer="94"/>
<wire x1="9.906" y1="-5.3975" x2="9.906" y2="-6.35" width="0.1524" layer="94"/>
<wire x1="9.906" y1="-6.35" x2="11.557" y2="-6.35" width="0.1524" layer="94"/>
<wire x1="11.557" y1="-6.35" x2="11.557" y2="-4.953" width="0.1524" layer="94"/>
<wire x1="11.557" y1="-4.953" x2="11.557" y2="-4.5085" width="0.1524" layer="94"/>
<wire x1="11.557" y1="-4.5085" x2="11.557" y2="-3.556" width="0.1524" layer="94"/>
<wire x1="11.557" y1="-3.556" x2="9.906" y2="-3.556" width="0.1524" layer="94"/>
<wire x1="10.6696" y1="-1.6338" x2="10.6696" y2="-3.4819" width="0.1524" layer="94"/>
<wire x1="9.906" y1="-5.3975" x2="11.557" y2="-4.5085" width="0.1524" layer="94"/>
<wire x1="-12.7" y1="-3.81" x2="-12.7" y2="2.54" width="0.1524" layer="94"/>
<wire x1="-12.7" y1="2.54" x2="-15.24" y2="2.54" width="0.1524" layer="94"/>
<wire x1="2.3102" y1="0.0138" x2="3.8352" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="4.0005" y2="0.762" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="4.572" y1="1.778" x2="5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="5.588" y1="1.778" x2="5.08" y2="1.778" width="0.1524" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="5.08" y2="-3.81" width="0.1524" layer="94"/>
<wire x1="5.08" y1="-3.81" x2="5.08" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="4.3815" y1="0.889" x2="4.0005" y2="0.762" width="0.254" layer="94"/>
<wire x1="5.08" y1="-3.81" x2="-12.7" y2="-3.81" width="0.1524" layer="94"/>
<wire x1="-0.2362" y1="0.0138" x2="1.2888" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="-15.24" y1="-2.54" x2="-16.3195" y2="0.762" width="0.254" layer="94"/>
<wire x1="-15.24" y1="-2.54" x2="-15.24" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="4.8438" y1="0.0138" x2="6.3688" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="7.0727" y1="0.0138" x2="8.5977" y2="0.0138" width="0.1524" layer="94"/>
<wire x1="5.08" y1="-10.16" x2="-15.24" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="-15.875" y1="-0.9525" x2="-17.78" y2="-0.9525" width="0.1524" layer="94"/>
<wire x1="-17.78" y1="-0.3175" x2="-17.78" y2="-1.524" width="0.1524" layer="94"/>
<wire x1="-15.24" y1="2.54" x2="-15.24" y2="1.0795" width="0.1524" layer="94"/>
<circle x="-10.16" y="1.27" radius="0.381" width="0.1524" layer="94"/>
<circle x="-5.08" y="1.27" radius="0.381" width="0.1524" layer="94"/>
<circle x="0" y="1.27" radius="0.381" width="0.1524" layer="94"/>
<circle x="5.08" y="1.27" radius="0.381" width="0.1524" layer="94"/>
<circle x="5.08" y="-10.16" radius="0.3175" width="0" layer="94"/>
<circle x="5.08" y="-3.81" radius="0.3175" width="0" layer="94"/>
<text x="-17.78" y="3.81" size="1.778" layer="95" rot="R180">&gt;PART</text>
<text x="-17.78" y="-3.81" size="1.778" layer="96" rot="R180">&gt;VALUE</text>
<text x="-17.78" y="-11.43" size="1.778" layer="96" rot="R180">&gt;FUNKTION</text>
<text x="-17.78" y="-6.35" size="1.778" layer="96" rot="R180">&gt;TYPE</text>
<text x="-17.78" y="-8.89" size="1.778" layer="96" rot="R180">&gt;HERSTELLER</text>
<text x="16.256" y="-0.889" size="1.778" layer="94" rot="MR0">H</text>
<text x="-20.3835" y="-2.159" size="2.1844" layer="94">T</text>
<rectangle x1="6.858" y1="-8.89" x2="8.382" y2="-4.8768" layer="94" rot="R180"/>
<rectangle x1="-10.922" y1="-8.89" x2="-9.398" y2="-4.8768" layer="94"/>
<rectangle x1="-5.842" y1="-8.89" x2="-4.318" y2="-4.8768" layer="94"/>
<rectangle x1="-0.762" y1="-8.89" x2="0.762" y2="-4.8768" layer="94"/>
<rectangle x1="4.318" y1="-8.89" x2="5.842" y2="-4.8768" layer="94"/>
<pin name="2" x="-10.16" y="-12.7" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="1" x="-10.16" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="4" x="-5.08" y="-12.7" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="3" x="-5.08" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="6" x="0" y="-12.7" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="5" x="0" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="N'" x="5.08" y="-12.7" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="N" x="5.08" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<polygon width="0.1524" layer="94">
<vertex x="10.668" y="-2.159"/>
<vertex x="11.1125" y="-2.6035"/>
<vertex x="10.2235" y="-2.6035"/>
</polygon>
</symbol>
<symbol name="LEITUNGSSCHUTZSCHALTER">
<wire x1="2.54" y1="1.524" x2="2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="0.4184" y1="1.0134" x2="-0.5512" y2="0.7246" width="0.1524" layer="94"/>
<wire x1="0.7072" y1="0.0438" x2="0.4184" y2="1.0134" width="0.1524" layer="94"/>
<wire x1="-0.2624" y1="-0.245" x2="-0.5512" y2="0.7246" width="0.1524" layer="94"/>
<wire x1="1.6872" y1="0.3224" x2="0.7072" y2="0.0438" width="0.1524" layer="94"/>
<wire x1="-0.2624" y1="-0.245" x2="-1.2422" y2="-0.5338" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="1.326" y2="1.5292" width="0.254" layer="94"/>
<wire x1="1.1433" y1="-1.1792" x2="1.9324" y2="-0.8666" width="0.1524" layer="94"/>
<text x="-2.54" y="2.54" size="1.778" layer="95" rot="R180">&gt;PART</text>
<text x="-2.54" y="0" size="1.778" layer="96" rot="R180">&gt;VALUE</text>
<text x="-2.54" y="-7.62" size="1.778" layer="96" rot="R180">&gt;FUNKTION</text>
<text x="-2.54" y="-2.54" size="1.778" layer="96" rot="R180">&gt;TYPE</text>
<text x="-2.54" y="-5.08" size="1.778" layer="96" rot="R180">&gt;HERSTELLER</text>
<pin name="1" x="2.54" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="2" x="2.54" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<polygon width="0.1524" layer="94">
<vertex x="0.3048" y="-1.4986"/>
<vertex x="1.143" y="-0.9144"/>
<vertex x="1.3208" y="-1.3716"/>
</polygon>
</symbol>
</symbols>
<devicesets>
<deviceset name="FEHLER-STROM-SCHUTZSCHALTER_4-POL" prefix="F">
<description>Fehlerstromschutzschalter (FI) 4-pol mit Prüftaster</description>
<gates>
<gate name="G$1" symbol="FEHLER-STROM-SCHUTZSCHALTER_4-POL" x="0" y="0"/>
</gates>
<devices>
<device name="" package="FEHLER-STROM-SCHUTZSCHALTER_4-POL">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="N" pad="N"/>
<connect gate="G$1" pin="N'" pad="N'"/>
</connects>
<technologies>
<technology name="">
<attribute name="FUNKTION" value="" constant="no"/>
<attribute name="HERSTELLER" value="" constant="no"/>
<attribute name="TYPE" value="" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LEITUNGSSCHUTZSCHALTER" prefix="F" uservalue="yes">
<description>Sicherung</description>
<gates>
<gate name="G$1" symbol="LEITUNGSSCHUTZSCHALTER" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LEITUNGSSCHUTZSCHALTER">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name="">
<attribute name="FUNKTION" value="" constant="no"/>
<attribute name="HERSTELLER" value="" constant="no"/>
<attribute name="TYPE" value="" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="e-schuetze-relais">
<description>&lt;b&gt;Schütze und Relais für Elektropläne&lt;/b&gt;&lt;p&gt;

Wenn das von Ihnen gewünschte Schütz in der vorliegenden Bibliothek nicht vorhanden ist, laden Sie
die Bibliothek &lt;u&gt;e-symbole.lbr&lt;/u&gt;, und starten Sie das User-Languag-Programm  &lt;u&gt;e-bauteil-erstellen.ulp&lt;/u&gt;.&lt;p&gt; 

&lt;author&gt;Autor librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL">
<description>Dummy</description>
<pad name="13" x="7.62" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="14" x="7.62" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="23" x="12.7" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="24" x="12.7" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="33" x="17.78" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="34" x="17.78" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="43" x="22.86" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="44" x="22.86" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="A1" x="0" y="-5.08" drill="0.2" diameter="0.6" shape="square"/>
<pad name="A2" x="0" y="5.08" drill="0.2" diameter="0.6" shape="square"/>
<text x="0" y="2.54" size="1.27" layer="25" font="vector">&gt;NAME</text>
<text x="0" y="-2.54" size="1.27" layer="27" font="vector">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL">
<wire x1="22.1042" y1="0.0064" x2="22.098" y2="0" width="0.1524" layer="94"/>
<wire x1="22.098" y1="0" x2="19.5772" y2="0.0064" width="0.1524" layer="94"/>
<wire x1="19.558" y1="0" x2="22.098" y2="0" width="0.1524" layer="94"/>
<wire x1="13.97" y1="0" x2="11.43" y2="0" width="0.1524" layer="94"/>
<wire x1="18.034" y1="0" x2="15.494" y2="0" width="0.1524" layer="94"/>
<wire x1="7.112" y1="0" x2="9.652" y2="0" width="0.1524" layer="94"/>
<wire x1="3.302" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>
<wire x1="3.175" y1="1.651" x2="0" y2="1.651" width="0.254" layer="94"/>
<wire x1="0" y1="1.651" x2="-3.175" y2="1.651" width="0.254" layer="94"/>
<wire x1="-3.175" y1="1.651" x2="-3.175" y2="-1.651" width="0.254" layer="94"/>
<wire x1="-3.175" y1="-1.651" x2="0" y2="-1.651" width="0.254" layer="94"/>
<wire x1="0" y1="-1.651" x2="3.175" y2="-1.651" width="0.254" layer="94"/>
<wire x1="3.175" y1="-1.651" x2="3.175" y2="1.651" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="1.651" width="0.1524" layer="94"/>
<wire x1="0" y1="-2.54" x2="0" y2="-1.651" width="0.1524" layer="94"/>
<wire x1="7.62" y1="-2.54" x2="6.35" y2="1.524" width="0.254" layer="94"/>
<wire x1="7.62" y1="2.54" x2="7.62" y2="1.778" width="0.1524" layer="94"/>
<wire x1="12.7" y1="-2.54" x2="11.43" y2="1.524" width="0.254" layer="94"/>
<wire x1="12.7" y1="2.54" x2="12.7" y2="1.778" width="0.1524" layer="94"/>
<wire x1="17.78" y1="-2.54" x2="16.51" y2="1.524" width="0.254" layer="94"/>
<wire x1="17.78" y1="2.54" x2="17.78" y2="1.778" width="0.1524" layer="94"/>
<wire x1="22.86" y1="-2.54" x2="21.59" y2="1.524" width="0.254" layer="94"/>
<wire x1="22.86" y1="2.54" x2="22.86" y2="1.778" width="0.1524" layer="94"/>
<text x="-3.81" y="2.54" size="1.778" layer="95" rot="R180">&gt;PART</text>
<text x="-3.81" y="0" size="1.778" layer="96" rot="R180">&gt;VALUE</text>
<text x="-3.81" y="-7.62" size="1.778" layer="96" rot="R180">&gt;FUNKTION</text>
<text x="-3.81" y="-2.54" size="1.778" layer="96" rot="R180">&gt;TYPE</text>
<text x="-3.81" y="-5.08" size="1.778" layer="96" rot="R180">&gt;HERSTELLER</text>
<pin name="14" x="7.62" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="13" x="7.62" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="24" x="12.7" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="23" x="12.7" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="34" x="17.78" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="33" x="17.78" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="44" x="22.86" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="43" x="22.86" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="A1" x="0" y="5.08" visible="pad" length="short" direction="in" rot="R270"/>
<pin name="A2" x="0" y="-5.08" visible="pad" length="short" direction="in" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL" prefix="K" uservalue="yes">
<description>Hilfsschütz mit 4 Schließern ohne Kontaktspiegel</description>
<gates>
<gate name="G$1" symbol="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL">
<connects>
<connect gate="G$1" pin="13" pad="13"/>
<connect gate="G$1" pin="14" pad="14"/>
<connect gate="G$1" pin="23" pad="23"/>
<connect gate="G$1" pin="24" pad="24"/>
<connect gate="G$1" pin="33" pad="33"/>
<connect gate="G$1" pin="34" pad="34"/>
<connect gate="G$1" pin="43" pad="43"/>
<connect gate="G$1" pin="44" pad="44"/>
<connect gate="G$1" pin="A1" pad="A1"/>
<connect gate="G$1" pin="A2" pad="A2"/>
</connects>
<technologies>
<technology name="">
<attribute name="FUNKTION" value="" constant="no"/>
<attribute name="HERSTELLER" value="" constant="no"/>
<attribute name="TYPE" value="" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="e-stromversorgungselemente">
<description>&lt;b&gt;Stromversorgungselemente für Elektropläne&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Autor librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="NETZTEIL_GLEICHSPANNUNG">
<description>Dummy</description>
<pad name="3" x="10.16" y="-5.08" drill="0.5" diameter="1" shape="square"/>
<pad name="4" x="10.16" y="-10.16" drill="0.5" diameter="1" shape="square"/>
<pad name="1" x="-10.16" y="-5.08" drill="0.5" diameter="1" shape="square"/>
<pad name="2" x="-10.16" y="-10.16" drill="0.5" diameter="1" shape="square"/>
</package>
</packages>
<symbols>
<symbol name="NETZTEIL_GLEICHSPANNUNG">
<wire x1="-5.08" y1="-5.08" x2="-5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="-5.08" y1="5.08" x2="5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="5.08" x2="5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="-5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="5.08" y2="5.08" width="0.254" layer="94"/>
<wire x1="1.524" y1="-1.778" x2="4.318" y2="-1.778" width="0.1524" layer="94"/>
<wire x1="1.524" y1="-2.286" x2="2.032" y2="-2.286" width="0.1524" layer="94"/>
<wire x1="2.667" y1="-2.286" x2="3.175" y2="-2.286" width="0.1524" layer="94"/>
<wire x1="3.81" y1="-2.286" x2="4.318" y2="-2.286" width="0.1524" layer="94"/>
<wire x1="-4.445" y1="1.397" x2="-4.318" y2="1.651" width="0.1524" layer="94"/>
<wire x1="-4.318" y1="1.651" x2="-3.81" y2="2.159" width="0.1524" layer="94" curve="-36.869898"/>
<wire x1="-3.81" y1="2.159" x2="-3.556" y2="2.159" width="0.1524" layer="94" curve="-53.130102"/>
<wire x1="-3.556" y1="2.159" x2="-3.048" y2="1.651" width="0.1524" layer="94" curve="-36.869898"/>
<wire x1="-3.048" y1="1.651" x2="-2.921" y2="1.397" width="0.1524" layer="94"/>
<wire x1="-1.397" y1="1.397" x2="-1.524" y2="1.143" width="0.1524" layer="94"/>
<wire x1="-1.524" y1="1.143" x2="-2.032" y2="0.635" width="0.1524" layer="94" curve="-36.869898"/>
<wire x1="-2.032" y1="0.635" x2="-2.286" y2="0.635" width="0.1524" layer="94" curve="-53.130102"/>
<wire x1="-2.286" y1="0.635" x2="-2.794" y2="1.143" width="0.1524" layer="94" curve="-36.869898"/>
<wire x1="-2.794" y1="1.143" x2="-2.921" y2="1.397" width="0.1524" layer="94"/>
<text x="-5.08" y="6.35" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-7.62" size="1.778" layer="96">&gt;VALUE</text>
<text x="-4.445" y="2.921" size="1.524" layer="96">&gt;EINGANGSSPANNUNG</text>
<text x="4.445" y="-3.048" size="1.524" layer="96" rot="R180">&gt;AUSGANGSSPANNUNG</text>
<text x="-5.08" y="-15.24" size="1.778" layer="96">&gt;FUNKTION</text>
<text x="-5.08" y="-10.16" size="1.778" layer="96">&gt;TYPE</text>
<text x="-5.08" y="-12.7" size="1.778" layer="96">&gt;HERSTELLER</text>
<pin name="1" x="-10.16" y="2.54" visible="pad" length="middle" direction="pas"/>
<pin name="2" x="-10.16" y="-2.54" visible="pad" length="middle" direction="pas"/>
<pin name="3" x="10.16" y="2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="4" x="10.16" y="-2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="NETZTEIL_GLEICHSPANNUNG" prefix="G" uservalue="yes">
<description>Netzteil, Gleichspannung</description>
<gates>
<gate name="G$1" symbol="NETZTEIL_GLEICHSPANNUNG" x="0" y="0"/>
</gates>
<devices>
<device name="" package="NETZTEIL_GLEICHSPANNUNG">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
</connects>
<technologies>
<technology name="">
<attribute name="AUSGANGSSPANNUNG" value="" constant="no"/>
<attribute name="EINGANGSSPANNUNG" value="" constant="no"/>
<attribute name="FUNKTION" value="" constant="no"/>
<attribute name="HERSTELLER" value="" constant="no"/>
<attribute name="TYPE" value="" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="e-messinstrumente">
<description>&lt;b&gt;Messinstrumente für Elektropläne&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Autor librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="ENERGIEZAEHLER/WATTSTUNDENZAEHLER">
<description>Dummy</description>
<pad name="1" x="-0.635" y="1.905" drill="0.2" diameter="0.6" shape="square"/>
<pad name="2" x="-0.635" y="-1.905" drill="0.2" diameter="0.6" shape="square"/>
<pad name="3" x="0.635" y="1.905" drill="0.2" diameter="0.6" shape="square"/>
<pad name="4" x="0.635" y="-1.905" drill="0.2" diameter="0.6" shape="square"/>
</package>
</packages>
<symbols>
<symbol name="ENERGIEZAEHLER/WATTSTUNDENZAEHLER">
<wire x1="-1.27" y1="0" x2="3.81" y2="0" width="0.254" layer="94"/>
<wire x1="3.81" y1="0" x2="3.81" y2="-5.08" width="0.254" layer="94"/>
<wire x1="3.81" y1="-5.08" x2="-1.27" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-5.08" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="-1.27" y1="2.54" x2="3.81" y2="2.54" width="0.254" layer="94"/>
<wire x1="3.81" y1="2.54" x2="3.81" y2="0" width="0.254" layer="94"/>
<text x="-0.635" y="-3.302" size="1.778" layer="94">Wh</text>
<text x="-2.54" y="2.54" size="1.778" layer="95" rot="R180">&gt;PART</text>
<text x="-2.54" y="0" size="1.778" layer="96" rot="R180">&gt;VALUE</text>
<text x="-2.54" y="-7.62" size="1.778" layer="96" rot="R180">&gt;FUNKTION</text>
<text x="-2.54" y="-2.54" size="1.778" layer="96" rot="R180">&gt;TYPE</text>
<text x="-2.54" y="-5.08" size="1.778" layer="96" rot="R180">&gt;HERSTELLER</text>
<pin name="U1" x="2.54" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="U2" x="2.54" y="-7.62" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="I1" x="0" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="I2" x="0" y="-7.62" visible="pad" length="short" direction="pas" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="ENERGIEZAEHLER/WATTSTUNDENZAEHLER" prefix="P" uservalue="yes">
<description>Energiezähler, Wattstundenzähler</description>
<gates>
<gate name="G$1" symbol="ENERGIEZAEHLER/WATTSTUNDENZAEHLER" x="0" y="0"/>
</gates>
<devices>
<device name="" package="ENERGIEZAEHLER/WATTSTUNDENZAEHLER">
<connects>
<connect gate="G$1" pin="I1" pad="1"/>
<connect gate="G$1" pin="I2" pad="2"/>
<connect gate="G$1" pin="U1" pad="3"/>
<connect gate="G$1" pin="U2" pad="4"/>
</connects>
<technologies>
<technology name="">
<attribute name="FUNKTION" value="" constant="no"/>
<attribute name="HERSTELLER" value="" constant="no"/>
<attribute name="TYPE" value="" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="e-klemmen">
<description>&lt;b&gt;Klemmen für Elektropläne&lt;/b&gt;&lt;p&gt;

Diese Bibliothek enthält Klemmen sowie Devices für Einspeisung und Erdung. Folgendes ist zu
beachten: &lt;p&gt;

&lt;b&gt;Einspeisungs-Devices&lt;/b&gt;&lt;p&gt;

Enthalten kein Package, da kein entsprechendes Bauteil existiert, das in einer Materialliste erscheinen sollte. &lt;p&gt;

&lt;b&gt;Erdungs-Devices&lt;/b&gt;&lt;p&gt;

Enthalten ein Package, da in einer Materialliste zumindest ein Bauteil mit Klemmmöglichkeit
erscheinen muss.&lt;p&gt;

&lt;b&gt;Klemmen&lt;/b&gt;&lt;p&gt;

Klemmennamen müssen im Schaltplan mit &lt;i&gt;X&lt;/i&gt; beginnen, damit Klemmenplan und Brückenplan richtig erzeugt werden. Deshalb ist ihr Prefix im Device auf X gesetzt. Bitte auch im Schaltplan keine
anderen Namen verwenden. Siehe auch: User-Language-Programm e-klemmenplan.ulp.&lt;p&gt;

&lt;b&gt;Brückenklemmen&lt;/b&gt;&lt;p&gt;

Brückenklemmen sind Klemmen, die zusätzlich zu den Drahtanschlüssen die Möglichkeit bieten, mit einem Brückenkamm eine Reihe von Klemmen zu verbinden. Mit dem User-Language-Programm
e-brueckenverwaltung.ulp werden solche Brücken definiert und als Liste ausgegeben. In dessen
Hilfe erfahren Sie Details. Dieses Programm setzt einige Dinge bei den verwendeten Bauteilen
voraus (nur wichtig, wenn Sie eigene Brückenklemmen definieren wollen):&lt;p&gt;

Der Device-Name muss &lt;i&gt;BRUECKE&lt;/i&gt; enthalten, andere Klemmen dürfen  &lt;i&gt;BRUECKE&lt;/i&gt;
nicht als Namensbestandteil enthalten.&lt;p&gt;

Die Pin-Namen der Klemmensymbole müssen 1 und 2 sein. Die damit verbundenen Pad-Namen
des Package müssen 1.1, 1.2; 2.1, 2.2 usw. sein. Dabei entspricht die Zahl vor dem Punkt dem
Gate-Namen (1, 2, 3..).&lt;p&gt;

Jedes Klemmensymbol muss gesondert definiert sein, da es für den Referenz-Text zwei Attribute verwendet, deren Platzhalter je Symbol unterschiedlich sind. Der Name der Attribute ist 51 und 52,
wenn es sich um Klemme 5 handelt. Der Parameter &lt;i&gt;display&lt;/i&gt; für diese Attribute (der im
Attribut-Bearbeitungsmenü im Feld &lt;i&gt;Anzeige&lt;/i&gt; eingestellt wird) muss im Schaltplan auf
&lt;i&gt;Off&lt;/i&gt; stehen, sonst werden die Referenz-Texte nicht an der richtigen Stelle dargestellt.
Wenn Sie das ULP zur Brückenverwaltung verwenden, geschieht das automatisch.&lt;p&gt;

&lt;p&gt;&lt;author&gt;Autor librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="EINSPEISUNG_3-N-PE">
<wire x1="-15.24" y1="5.08" x2="15.24" y2="5.08" width="0.254" layer="94"/>
<wire x1="15.24" y1="5.08" x2="15.24" y2="-5.08" width="0.254" layer="94"/>
<wire x1="15.24" y1="-5.08" x2="-15.24" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-15.24" y1="-5.08" x2="-15.24" y2="5.08" width="0.254" layer="94"/>
<text x="-13.97" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
<text x="-15.24" y="-7.62" size="1.778" layer="94">EINSPEISUNG</text>
<text x="9.271" y="2.921" size="1.778" layer="94" rot="R270">PE</text>
<text x="4.191" y="2.921" size="1.778" layer="94" rot="R270">N</text>
<text x="-0.889" y="2.921" size="1.778" layer="94" rot="R270">L3</text>
<text x="-5.969" y="2.921" size="1.778" layer="94" rot="R270">L2</text>
<text x="-11.049" y="2.921" size="1.778" layer="94" rot="R270">L1</text>
<pin name="L1-EXT" x="-10.16" y="7.62" visible="off" length="short" direction="sup" rot="R270"/>
<pin name="L2-EXT" x="-5.08" y="7.62" visible="off" length="short" direction="sup" rot="R270"/>
<pin name="L3-EXT" x="0" y="7.62" visible="off" length="short" direction="sup" rot="R270"/>
<pin name="N-EXT" x="5.08" y="7.62" visible="off" length="short" direction="sup" rot="R270"/>
<pin name="PE-EXT" x="10.16" y="7.62" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="EINSPEISUNG_3-N-PE" prefix="ESP" uservalue="yes">
<description>Einspeisung für 3 Phasen mit N und PE</description>
<gates>
<gate name="G$1" symbol="EINSPEISUNG_3-N-PE" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="ESP32-DEVKITV1">
<packages>
<package name="ESP32-DEVKITV1">
<pad name="1" x="-22.87" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="2" x="-20.33" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="3" x="-17.79" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="4" x="-15.25" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="5" x="-12.71" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="6" x="-10.17" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="7" x="-7.63" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="8" x="-5.09" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="9" x="-2.55" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="10" x="-0.01" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="11" x="2.53" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="12" x="5.07" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="13" x="7.61" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="14" x="10.15" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="15" x="12.69" y="-13.54" drill="1" diameter="1.9304"/>
<pad name="30" x="-22.87" y="11.23" drill="1" diameter="1.9304"/>
<pad name="29" x="-20.33" y="11.23" drill="1" diameter="1.9304"/>
<pad name="28" x="-17.79" y="11.23" drill="1" diameter="1.9304"/>
<pad name="27" x="-15.25" y="11.23" drill="1" diameter="1.9304"/>
<pad name="26" x="-12.71" y="11.23" drill="1" diameter="1.9304"/>
<pad name="25" x="-10.17" y="11.23" drill="1" diameter="1.9304"/>
<pad name="24" x="-7.63" y="11.23" drill="1" diameter="1.9304"/>
<pad name="23" x="-5.09" y="11.23" drill="1" diameter="1.9304"/>
<pad name="22" x="-2.55" y="11.23" drill="1" diameter="1.9304"/>
<pad name="21" x="-0.01" y="11.23" drill="1" diameter="1.9304"/>
<pad name="20" x="2.53" y="11.23" drill="1" diameter="1.9304"/>
<pad name="19" x="5.07" y="11.23" drill="1" diameter="1.9304"/>
<pad name="18" x="7.61" y="11.23" drill="1" diameter="1.9304"/>
<pad name="17" x="10.15" y="11.23" drill="1" diameter="1.9304"/>
<pad name="16" x="12.69" y="11.23" drill="1" diameter="1.9304"/>
<text x="-22.21" y="-11.2" size="1.016" layer="21" rot="R90">3V3</text>
<text x="-19.67" y="-11.2" size="1.016" layer="21" rot="R90">GND</text>
<text x="-17.13" y="-11.2" size="1.016" layer="21" rot="R90">IO15</text>
<text x="-14.59" y="-11.2" size="1.016" layer="21" rot="R90">IO2</text>
<text x="-12.05" y="-11.2" size="1.016" layer="21" rot="R90">IO4</text>
<text x="-9.51" y="-11.2" size="1.016" layer="21" rot="R90">IO16</text>
<text x="-6.97" y="-11.2" size="1.016" layer="21" rot="R90">IO17</text>
<text x="-4.43" y="-11.2" size="1.016" layer="21" rot="R90">IO5</text>
<text x="-1.89" y="-11.2" size="1.016" layer="21" rot="R90">IO18</text>
<text x="0.65" y="-11.2" size="1.016" layer="21" rot="R90">IO19</text>
<text x="3.19" y="-11.2" size="1.016" layer="21" rot="R90">IO21</text>
<text x="5.73" y="-11.2" size="1.016" layer="21" rot="R90">IO3</text>
<text x="8.27" y="-11.2" size="1.016" layer="21" rot="R90">IO1</text>
<text x="10.81" y="-11.2" size="1.016" layer="21" rot="R90">IO22</text>
<text x="13.35" y="-11.2" size="1.016" layer="21" rot="R90">IO23</text>
<text x="-22.19" y="6.52" size="1.016" layer="21" rot="R90">VIN</text>
<text x="-19.65" y="6.52" size="1.016" layer="21" rot="R90">GND</text>
<text x="-17.11" y="6.52" size="1.016" layer="21" rot="R90">IO13</text>
<text x="-14.57" y="6.52" size="1.016" layer="21" rot="R90">IO12</text>
<text x="-12.03" y="6.52" size="1.016" layer="21" rot="R90">IO14</text>
<text x="-9.49" y="6.52" size="1.016" layer="21" rot="R90">IO27</text>
<text x="-6.95" y="6.52" size="1.016" layer="21" rot="R90">IO26</text>
<text x="-4.41" y="6.52" size="1.016" layer="21" rot="R90">IO25</text>
<text x="-1.87" y="6.52" size="1.016" layer="21" rot="R90">IO33</text>
<text x="0.67" y="6.52" size="1.016" layer="21" rot="R90">IO32</text>
<text x="3.21" y="6.52" size="1.016" layer="21" rot="R90">IO35</text>
<text x="5.75" y="6.52" size="1.016" layer="21" rot="R90">IO34</text>
<text x="8.29" y="6.52" size="1.016" layer="21" rot="R90">VN</text>
<text x="10.83" y="6.52" size="1.016" layer="21" rot="R90">VP</text>
<text x="13.37" y="6.52" size="1.016" layer="21" rot="R90">EN</text>
<text x="-4.93" y="-3.18" size="1.9304" layer="21">ESP32-Devkit V1</text>
<wire x1="-33" y1="12.7" x2="19" y2="12.7" width="0.254" layer="21"/>
<wire x1="19" y1="12.7" x2="19" y2="-15.2" width="0.254" layer="21"/>
<wire x1="19" y1="-15.2" x2="-33" y2="-15.2" width="0.254" layer="21"/>
<wire x1="-33" y1="-15.2" x2="-33" y2="12.7" width="0.254" layer="21"/>
<text x="-24.13" y="13.97" size="1.27" layer="21">&gt;NAME</text>
<text x="5" y="-17.24" size="1.27" layer="27">ESP32-DEVKITV1</text>
</package>
</packages>
<symbols>
<symbol name="ESP32-DEVKITV1">
<wire x1="-25.4" y1="-12.7" x2="-25.4" y2="12.7" width="0.254" layer="94"/>
<wire x1="-25.4" y1="12.7" x2="16" y2="12.7" width="0.254" layer="94"/>
<wire x1="16" y1="12.7" x2="16" y2="-12.7" width="0.254" layer="94"/>
<wire x1="16" y1="-12.7" x2="-25.4" y2="-12.7" width="0.254" layer="94"/>
<pin name="3V3" x="-22.86" y="-17.78" length="middle" rot="R90"/>
<pin name="GND" x="-20.32" y="-17.78" length="middle" rot="R90"/>
<pin name="IO15" x="-17.78" y="-17.78" length="middle" rot="R90"/>
<pin name="IO2" x="-15.24" y="-17.78" length="middle" rot="R90"/>
<pin name="IO4" x="-12.7" y="-17.78" length="middle" rot="R90"/>
<pin name="IO16" x="-10.16" y="-17.78" length="middle" rot="R90"/>
<pin name="IO17" x="-7.62" y="-17.78" length="middle" rot="R90"/>
<pin name="IO5" x="-5.08" y="-17.78" length="middle" rot="R90"/>
<pin name="IO18" x="-2.54" y="-17.78" length="middle" rot="R90"/>
<pin name="IO19" x="0" y="-17.78" length="middle" rot="R90"/>
<pin name="IO21" x="2.54" y="-17.78" length="middle" rot="R90"/>
<pin name="IO3" x="5.08" y="-17.78" length="middle" rot="R90"/>
<pin name="IO1" x="7.62" y="-17.78" length="middle" rot="R90"/>
<pin name="IO22" x="10.16" y="-17.78" length="middle" rot="R90"/>
<pin name="IO23" x="12.7" y="-17.78" length="middle" rot="R90"/>
<pin name="EN" x="12.7" y="17.78" length="middle" rot="R270"/>
<pin name="VP" x="10.16" y="17.78" length="middle" rot="R270"/>
<pin name="VN" x="7.62" y="17.78" length="middle" rot="R270"/>
<pin name="IO34" x="5.08" y="17.78" length="middle" rot="R270"/>
<pin name="IO35" x="2.54" y="17.78" length="middle" rot="R270"/>
<pin name="IO32" x="0" y="17.78" length="middle" rot="R270"/>
<pin name="IO33" x="-2.54" y="17.78" length="middle" rot="R270"/>
<pin name="IO25" x="-5.08" y="17.78" length="middle" rot="R270"/>
<pin name="IO26" x="-7.62" y="17.78" length="middle" rot="R270"/>
<pin name="IO27" x="-10.16" y="17.78" length="middle" rot="R270"/>
<pin name="IO14" x="-12.7" y="17.78" length="middle" rot="R270"/>
<pin name="IO12" x="-15.24" y="17.78" length="middle" rot="R270"/>
<pin name="IO13" x="-17.78" y="17.78" length="middle" rot="R270"/>
<pin name="GND1" x="-20.32" y="17.78" length="middle" rot="R270"/>
<pin name="VIN" x="-22.86" y="17.78" length="middle" rot="R270"/>
<text x="-26.67" y="5.08" size="1.27" layer="95" rot="R90">&gt;NAME</text>
<text x="18.4" y="-12.7" size="1.27" layer="96" rot="R90">ESP32-DEVKITV1</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="ESP32DEVKITV1">
<gates>
<gate name="G$1" symbol="ESP32-DEVKITV1" x="0" y="0"/>
</gates>
<devices>
<device name="" package="ESP32-DEVKITV1">
<connects>
<connect gate="G$1" pin="3V3" pad="1"/>
<connect gate="G$1" pin="EN" pad="16"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="GND1" pad="28"/>
<connect gate="G$1" pin="IO1" pad="13"/>
<connect gate="G$1" pin="IO12" pad="27"/>
<connect gate="G$1" pin="IO13" pad="28"/>
<connect gate="G$1" pin="IO14" pad="26"/>
<connect gate="G$1" pin="IO15" pad="3"/>
<connect gate="G$1" pin="IO16" pad="6"/>
<connect gate="G$1" pin="IO17" pad="7"/>
<connect gate="G$1" pin="IO18" pad="9"/>
<connect gate="G$1" pin="IO19" pad="10"/>
<connect gate="G$1" pin="IO2" pad="4"/>
<connect gate="G$1" pin="IO21" pad="11"/>
<connect gate="G$1" pin="IO22" pad="14"/>
<connect gate="G$1" pin="IO23" pad="15"/>
<connect gate="G$1" pin="IO25" pad="23"/>
<connect gate="G$1" pin="IO26" pad="24"/>
<connect gate="G$1" pin="IO27" pad="24"/>
<connect gate="G$1" pin="IO3" pad="12"/>
<connect gate="G$1" pin="IO32" pad="21"/>
<connect gate="G$1" pin="IO33" pad="22"/>
<connect gate="G$1" pin="IO34" pad="19"/>
<connect gate="G$1" pin="IO35" pad="20"/>
<connect gate="G$1" pin="IO4" pad="5"/>
<connect gate="G$1" pin="IO5" pad="8"/>
<connect gate="G$1" pin="VIN" pad="30"/>
<connect gate="G$1" pin="VN" pad="18"/>
<connect gate="G$1" pin="VP" pad="17"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="App Store">
<description>&lt;b&gt;DIY Modules for Arduino, Raspberry Pi, CubieBoard etc.&lt;/b&gt;
&lt;br&gt;&lt;br&gt;
The library contains a list of symbols and footprints for popular, cheap and easy-to-use electronic modules.&lt;br&gt;
The modules are intend to work with microprocessor-based platforms such as &lt;a href="http://arduino.cc"&gt;Arduino&lt;/a&gt;, &lt;a href="http://raspberrypi.org/"&gt;Raspberry Pi&lt;/a&gt;, &lt;a href="http://cubieboard.org/"&gt;CubieBoard&lt;/a&gt;, &lt;a href="http://beagleboard.org/"&gt;BeagleBone&lt;/a&gt; and many others. There are many manufacturers of the modules in the world. Almost all of them can be bought on &lt;a href="ebay.com"&gt;ebay.com&lt;/a&gt;.&lt;br&gt;
&lt;br&gt;
By using this library, you can design a PCB for devices created with usage of modules. Even if you do not need to create PCB design, you can also use the library to quickly document your work by drawing schematics of devices built by you.&lt;br&gt;
&lt;br&gt;
The latest version, examples, photos and much more can be found at: &lt;b&gt;&lt;a href="http://diymodules.org/eagle"&gt;diymodules.org/eagle&lt;/a&gt;&lt;/b&gt;&lt;br&gt;&lt;br&gt;
Comments, suggestions and bug reports please send to: &lt;b&gt;&lt;a href="mailto:eagle@diymodules.org"&gt;eagle@diymodules.org&lt;/b&gt;&lt;/a&gt;&lt;br&gt;&lt;br&gt;
&lt;i&gt;Version: 1.8.0 (2017-Jul-02)&lt;/i&gt;&lt;br&gt;
&lt;i&gt;Created by: Miroslaw Brudnowski&lt;/i&gt;&lt;br&gt;&lt;br&gt;
&lt;i&gt;Released under the Creative Commons Attribution 4.0 International License: &lt;a href="http://creativecommons.org/licenses/by/4.0"&gt;http://creativecommons.org/licenses/by/4.0&lt;/a&gt;&lt;/i&gt;
&lt;br&gt;&lt;br&gt;
&lt;center&gt;
&lt;a href="http://diymodules.org/eagle"&gt;&lt;img src="http://www.diymodules.org/img/diymodules-lbr-image.php?v=1.8.0" alt="DIYmodules.org"&gt;&lt;/a&gt;
&lt;/center&gt;</description>
<packages>
<package name="RELAY-1-SRD#RC">
<description>&lt;b&gt;1-Channel Relay Module&lt;/b&gt; based on &lt;b&gt;SRD-05VDC-SL-C&lt;/b&gt; relay&lt;br&gt;
Footprint with relays connector</description>
<wire x1="-26.67" y1="14.224" x2="26.67" y2="14.224" width="0.127" layer="21"/>
<wire x1="26.67" y1="14.224" x2="26.67" y2="-14.224" width="0.127" layer="21"/>
<wire x1="26.67" y1="-14.224" x2="-26.67" y2="-14.224" width="0.127" layer="21"/>
<wire x1="-26.67" y1="-14.224" x2="-26.67" y2="14.224" width="0.127" layer="21"/>
<hole x="-23.368" y="10.922" drill="3.2"/>
<hole x="23.368" y="10.922" drill="3.2"/>
<hole x="23.368" y="-10.922" drill="3.2"/>
<hole x="-23.368" y="-10.922" drill="3.2"/>
<pad name="J1.1" x="25.4" y="2.54" drill="1" shape="square"/>
<pad name="J1.2" x="25.4" y="0" drill="1"/>
<pad name="J1.3" x="25.4" y="-2.54" drill="1"/>
<wire x1="24.13" y1="3.175" x2="24.765" y2="3.81" width="0.127" layer="21"/>
<wire x1="24.765" y1="3.81" x2="26.035" y2="3.81" width="0.127" layer="21"/>
<wire x1="26.035" y1="3.81" x2="26.67" y2="3.175" width="0.127" layer="21"/>
<wire x1="26.67" y1="1.905" x2="26.035" y2="1.27" width="0.127" layer="21"/>
<wire x1="26.035" y1="1.27" x2="26.67" y2="0.635" width="0.127" layer="21"/>
<wire x1="26.67" y1="-0.635" x2="26.035" y2="-1.27" width="0.127" layer="21"/>
<wire x1="26.035" y1="-1.27" x2="26.67" y2="-1.905" width="0.127" layer="21"/>
<wire x1="26.67" y1="-3.175" x2="26.035" y2="-3.81" width="0.127" layer="21"/>
<wire x1="24.765" y1="-3.81" x2="24.13" y2="-3.175" width="0.127" layer="21"/>
<wire x1="24.13" y1="-3.175" x2="24.13" y2="-1.905" width="0.127" layer="21"/>
<wire x1="24.13" y1="-1.905" x2="24.765" y2="-1.27" width="0.127" layer="21"/>
<wire x1="24.765" y1="-1.27" x2="24.13" y2="-0.635" width="0.127" layer="21"/>
<wire x1="24.13" y1="-0.635" x2="24.13" y2="0.635" width="0.127" layer="21"/>
<wire x1="24.13" y1="0.635" x2="24.765" y2="1.27" width="0.127" layer="21"/>
<wire x1="24.765" y1="1.27" x2="24.13" y2="1.905" width="0.127" layer="21"/>
<wire x1="24.13" y1="1.905" x2="24.13" y2="3.175" width="0.127" layer="21"/>
<wire x1="-22.225" y1="7.62" x2="-15.875" y2="7.62" width="0.127" layer="21"/>
<wire x1="-15.875" y1="7.62" x2="-14.605" y2="7.62" width="0.127" layer="21"/>
<wire x1="-14.605" y1="7.62" x2="-14.605" y2="2.54" width="0.127" layer="21"/>
<wire x1="-14.605" y1="2.54" x2="-14.605" y2="-2.54" width="0.127" layer="21"/>
<wire x1="-14.605" y1="-2.54" x2="-22.225" y2="-2.54" width="0.127" layer="21"/>
<wire x1="-22.225" y1="-2.54" x2="-22.225" y2="2.54" width="0.127" layer="21"/>
<wire x1="-22.225" y1="2.54" x2="-22.225" y2="7.62" width="0.127" layer="21"/>
<wire x1="-14.605" y1="2.54" x2="-22.225" y2="2.54" width="0.127" layer="21"/>
<wire x1="-22.225" y1="-2.54" x2="-22.225" y2="-7.62" width="0.127" layer="21"/>
<wire x1="-14.605" y1="-7.62" x2="-14.605" y2="-2.54" width="0.127" layer="21"/>
<wire x1="-22.225" y1="-7.62" x2="-15.875" y2="-7.62" width="0.127" layer="21"/>
<wire x1="-15.875" y1="-7.62" x2="-14.605" y2="-7.62" width="0.127" layer="21"/>
<wire x1="-15.875" y1="7.62" x2="-15.875" y2="-7.62" width="0.127" layer="21"/>
<circle x="-15.24" y="5.08" radius="0.381" width="0.127" layer="21"/>
<circle x="-15.24" y="0" radius="0.381" width="0.127" layer="21"/>
<circle x="-15.24" y="-5.08" radius="0.381" width="0.127" layer="21"/>
<text x="0" y="15.24" size="1.27" layer="25" align="bottom-center">&gt;NAME</text>
<text x="0" y="-15.24" size="1.27" layer="27" align="top-center">&gt;VALUE</text>
<wire x1="26.035" y1="-3.81" x2="24.765" y2="-3.81" width="0.127" layer="21"/>
<pad name="J2.1" x="13.335" y="6.985" drill="1" shape="square"/>
<pad name="J2.2" x="15.875" y="6.985" drill="1"/>
<wire x1="12.065" y1="7.62" x2="12.7" y2="8.255" width="0.127" layer="21"/>
<wire x1="12.7" y1="8.255" x2="13.97" y2="8.255" width="0.127" layer="21"/>
<wire x1="13.97" y1="8.255" x2="14.605" y2="7.62" width="0.127" layer="21"/>
<wire x1="14.605" y1="7.62" x2="15.24" y2="8.255" width="0.127" layer="21"/>
<wire x1="15.24" y1="8.255" x2="16.51" y2="8.255" width="0.127" layer="21"/>
<wire x1="16.51" y1="8.255" x2="17.145" y2="7.62" width="0.127" layer="21"/>
<wire x1="17.145" y1="7.62" x2="17.145" y2="6.35" width="0.127" layer="21"/>
<wire x1="17.145" y1="6.35" x2="16.51" y2="5.715" width="0.127" layer="21"/>
<wire x1="16.51" y1="5.715" x2="15.24" y2="5.715" width="0.127" layer="21"/>
<wire x1="15.24" y1="5.715" x2="14.605" y2="6.35" width="0.127" layer="21"/>
<wire x1="14.605" y1="6.35" x2="13.97" y2="5.715" width="0.127" layer="21"/>
<wire x1="13.97" y1="5.715" x2="12.7" y2="5.715" width="0.127" layer="21"/>
<wire x1="12.7" y1="5.715" x2="12.065" y2="6.35" width="0.127" layer="21"/>
<wire x1="12.065" y1="6.35" x2="12.065" y2="7.62" width="0.127" layer="21"/>
<pad name="P1.1" x="-19.05" y="5.08" drill="1.2" diameter="2.5"/>
<pad name="P1.2" x="-19.05" y="0" drill="1.2" diameter="2.5"/>
<pad name="P1.3" x="-19.05" y="-5.08" drill="1.2" diameter="2.5"/>
</package>
</packages>
<symbols>
<symbol name="RELAY-1-SRD#RC">
<description>&lt;b&gt;1-Channel Relay Module&lt;/b&gt; based on &lt;b&gt;SRD-05VDC-SL-C&lt;/b&gt; relay&lt;br&gt;
Symbol with relays pins</description>
<pin name="VCC.1" x="22.86" y="-5.08" length="middle" direction="pwr" rot="R180"/>
<pin name="IN1" x="22.86" y="-2.54" length="middle" rot="R180"/>
<pin name="GND" x="22.86" y="0" length="middle" direction="pwr" rot="R180"/>
<pin name="VCC.2" x="22.86" y="5.08" length="middle" direction="pwr" rot="R180"/>
<pin name="JD-VCC" x="22.86" y="7.62" length="middle" rot="R180"/>
<wire x1="-17.78" y1="10.16" x2="17.78" y2="10.16" width="0.254" layer="94"/>
<wire x1="17.78" y1="10.16" x2="17.78" y2="-10.16" width="0.254" layer="94"/>
<wire x1="17.78" y1="-10.16" x2="-17.78" y2="-10.16" width="0.254" layer="94"/>
<text x="-17.78" y="15.24" size="1.778" layer="95">&gt;NAME</text>
<text x="-17.78" y="12.7" size="1.778" layer="95">&gt;VALUE</text>
<wire x1="-17.78" y1="-10.16" x2="-17.78" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-17.78" y1="-7.62" x2="-17.78" y2="-2.54" width="0.254" layer="94"/>
<wire x1="-17.78" y1="-2.54" x2="-17.78" y2="2.54" width="0.254" layer="94"/>
<wire x1="-17.78" y1="2.54" x2="-17.78" y2="7.62" width="0.254" layer="94"/>
<wire x1="-17.78" y1="7.62" x2="-17.78" y2="10.16" width="0.254" layer="94"/>
<pin name="REL.1.NO" x="-22.86" y="5.08" length="middle"/>
<pin name="REL.1.COM" x="-22.86" y="0" length="middle"/>
<pin name="REL.1.NC" x="-22.86" y="-5.08" length="middle"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="RELAY-1-SRD#RC">
<description>&lt;b&gt;1-Channel Relay Module&lt;/b&gt; based on &lt;b&gt;SRD-05VDC-SL-C&lt;/b&gt; relays&lt;br&gt;
Symbol with relays pins
&lt;p&gt;&lt;b&gt;&lt;a href="http://www.ebay.com/sch/one+channel+isolated+relay+5v"&gt;Click here to find device on ebay.com&lt;/a&gt;&lt;/b&gt;&lt;/p&gt;

&lt;p&gt;&lt;img alt="photo" src="http://www.diymodules.org/img/device-photo.php?name=RELAY-1-SRD"&gt;&lt;/p&gt;</description>
<gates>
<gate name="G$1" symbol="RELAY-1-SRD#RC" x="0" y="0"/>
</gates>
<devices>
<device name="" package="RELAY-1-SRD#RC">
<connects>
<connect gate="G$1" pin="GND" pad="J1.1"/>
<connect gate="G$1" pin="IN1" pad="J1.2"/>
<connect gate="G$1" pin="JD-VCC" pad="J2.1"/>
<connect gate="G$1" pin="REL.1.COM" pad="P1.2"/>
<connect gate="G$1" pin="REL.1.NC" pad="P1.3"/>
<connect gate="G$1" pin="REL.1.NO" pad="P1.1"/>
<connect gate="G$1" pin="VCC.1" pad="J1.3"/>
<connect gate="G$1" pin="VCC.2" pad="J2.2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="EVSEWB">
<packages>
<package name="EVSEWB">
<wire x1="-3.81" y1="6.35" x2="-3.81" y2="-10.16" width="0.127" layer="21"/>
<wire x1="-3.81" y1="-10.16" x2="11.43" y2="-10.16" width="0.127" layer="21"/>
<wire x1="11.43" y1="-10.16" x2="11.43" y2="6.35" width="0.127" layer="21"/>
<wire x1="11.43" y1="6.35" x2="-3.81" y2="6.35" width="0.127" layer="21"/>
<pad name="L" x="0" y="5.08" drill="0.8" shape="square"/>
<pad name="N" x="2.54" y="5.08" drill="0.8" shape="square"/>
<pad name="PE" x="5.08" y="5.08" drill="0.8" shape="square"/>
<pad name="REL" x="7.62" y="5.08" drill="0.8" shape="square"/>
<pad name="AN" x="-2.54" y="-8.89" drill="0.8" shape="square"/>
<pad name="LED" x="0" y="-8.89" drill="0.8" shape="square"/>
<pad name="GND" x="2.54" y="-8.89" drill="0.8" shape="square"/>
<pad name="5V" x="5.08" y="-8.89" drill="0.8" shape="square"/>
<pad name="PP" x="7.62" y="-8.89" drill="0.8" shape="square"/>
<pad name="CP" x="10.16" y="-8.89" drill="0.8" shape="square"/>
<pad name="TX" x="-2.54" y="-1.27" drill="0.8" shape="square"/>
<pad name="RX" x="-2.54" y="1.27" drill="0.8" shape="square"/>
</package>
</packages>
<symbols>
<symbol name="EVSEWB">
<wire x1="-10.16" y1="12.7" x2="10.16" y2="12.7" width="0.254" layer="94"/>
<wire x1="10.16" y1="12.7" x2="10.16" y2="-12.7" width="0.254" layer="94"/>
<wire x1="10.16" y1="-12.7" x2="-10.16" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-12.7" x2="-10.16" y2="12.7" width="0.254" layer="94"/>
<text x="-3.81" y="-0.762" size="1.27" layer="94">EVSE WB</text>
<pin name="AN" x="-15.24" y="2.54" length="middle"/>
<pin name="LED" x="-15.24" y="0" length="middle"/>
<pin name="GND" x="-15.24" y="-2.54" length="middle"/>
<pin name="5V" x="-15.24" y="-5.08" length="middle"/>
<pin name="PP" x="-15.24" y="-7.62" length="middle"/>
<pin name="CP" x="-15.24" y="-10.16" length="middle"/>
<pin name="L" x="15.24" y="-7.62" length="middle" rot="R180"/>
<pin name="N" x="15.24" y="-2.54" length="middle" rot="R180"/>
<pin name="PE" x="15.24" y="2.54" length="middle" rot="R180"/>
<pin name="REL" x="15.24" y="7.62" length="middle" rot="R180"/>
<pin name="RXD" x="-15.24" y="7.62" length="middle"/>
<pin name="TXD" x="-15.24" y="10.16" length="middle"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="EVSEWB">
<gates>
<gate name="G$1" symbol="EVSEWB" x="0" y="0"/>
</gates>
<devices>
<device name="" package="EVSEWB">
<connects>
<connect gate="G$1" pin="5V" pad="5V"/>
<connect gate="G$1" pin="AN" pad="AN"/>
<connect gate="G$1" pin="CP" pad="CP"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="L" pad="L"/>
<connect gate="G$1" pin="LED" pad="LED"/>
<connect gate="G$1" pin="N" pad="N"/>
<connect gate="G$1" pin="PE" pad="PE"/>
<connect gate="G$1" pin="PP" pad="PP"/>
<connect gate="G$1" pin="REL" pad="REL"/>
<connect gate="G$1" pin="RXD" pad="RX"/>
<connect gate="G$1" pin="TXD" pad="TX"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="con-amp">
<description>&lt;b&gt;AMP Connectors&lt;/b&gt;&lt;p&gt;
RJ45 Jack connectors&lt;br&gt;
 Based on the previous libraris:
 &lt;ul&gt;
 &lt;li&gt;amp.lbr
 &lt;li&gt;amp-j.lbr
 &lt;li&gt;amp-mta.lbr
 &lt;li&gt;amp-nlok.lbr
 &lt;li&gt;amp-sim.lbr
 &lt;li&gt;amp-micro-match.lbr
 &lt;/ul&gt;
 Sources :
 &lt;ul&gt;
 &lt;li&gt;Catalog 82066 Revised 11-95 
 &lt;li&gt;Product Guide 296785 Rev. 8-99
 &lt;li&gt;Product Guide CD-ROM 1999
 &lt;li&gt;www.amp.com
 &lt;/ul&gt;
 &lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="1X7MTA">
<description>&lt;b&gt;AMP MTA connector&lt;/b&gt;</description>
<wire x1="13.97" y1="6.35" x2="13.97" y2="3.81" width="0.4064" layer="21"/>
<wire x1="13.97" y1="3.81" x2="13.97" y2="-3.81" width="0.4064" layer="21"/>
<wire x1="-13.97" y1="-3.81" x2="-13.97" y2="3.81" width="0.4064" layer="21"/>
<wire x1="-13.97" y1="3.81" x2="-13.97" y2="6.35" width="0.4064" layer="21"/>
<wire x1="-13.97" y1="6.35" x2="13.97" y2="6.35" width="0.4064" layer="21"/>
<wire x1="-13.97" y1="3.81" x2="13.97" y2="3.81" width="0.3048" layer="21"/>
<wire x1="-13.97" y1="-3.81" x2="13.97" y2="-3.81" width="0.4064" layer="21"/>
<pad name="3" x="3.9624" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="2" x="7.9248" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="1" x="11.8872" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="4" x="0" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="5" x="-3.9624" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="6" x="-7.9248" y="0" drill="1.6764" shape="long" rot="R90"/>
<pad name="7" x="-11.8872" y="0" drill="1.6764" shape="long" rot="R90"/>
<text x="-3.5814" y="-7.0612" size="1.778" layer="25">&gt;NAME</text>
<text x="-13.1763" y="4.4508" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="11.3792" y1="-0.508" x2="12.3952" y2="0.508" layer="21"/>
<rectangle x1="7.4168" y1="-0.508" x2="8.4328" y2="0.508" layer="21"/>
<rectangle x1="3.4544" y1="-0.508" x2="4.4704" y2="0.508" layer="21"/>
<rectangle x1="-0.508" y1="-0.508" x2="0.508" y2="0.508" layer="21"/>
<rectangle x1="-4.4704" y1="-0.508" x2="-3.4544" y2="0.508" layer="21"/>
<rectangle x1="-8.4328" y1="-0.508" x2="-7.4168" y2="0.508" layer="21"/>
<rectangle x1="-12.3952" y1="-0.508" x2="-11.3792" y2="0.508" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="MTA-07">
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<wire x1="16.51" y1="-1.905" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<wire x1="16.51" y1="-1.905" x2="16.51" y2="1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="16.51" y2="1.27" width="0.254" layer="94"/>
<circle x="0" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="2.54" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="5.08" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="7.62" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="10.16" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="12.7" y="0" radius="0.635" width="0.254" layer="94"/>
<circle x="15.24" y="0" radius="0.635" width="0.254" layer="94"/>
<text x="17.78" y="0" size="1.778" layer="95">&gt;NAME</text>
<text x="17.78" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
<text x="-2.54" y="-1.27" size="1.27" layer="95">1</text>
<pin name="1" x="0" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="2" x="2.54" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="3" x="5.08" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="4" x="7.62" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="5" x="10.16" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="6" x="12.7" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="7" x="15.24" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="MTA07-156" prefix="J" uservalue="yes">
<description>&lt;b&gt;AMP connector&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="MTA-07" x="-7.62" y="0"/>
</gates>
<devices>
<device name="" package="1X7MTA">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="frames">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="DINA4_L">
<frame x1="0" y1="0" x2="264.16" y2="180.34" columns="4" rows="4" layer="94" border-left="no" border-top="no" border-right="no" border-bottom="no"/>
</symbol>
<symbol name="DOCFIELD">
<wire x1="0" y1="0" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="87.63" y2="15.24" width="0.1016" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="71.12" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="0" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="87.63" y1="5.08" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="101.6" y1="5.08" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="0" y1="15.24" x2="0" y2="22.86" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="22.86" x2="101.6" y2="15.24" width="0.1016" layer="94"/>
<text x="1.27" y="1.27" size="2.54" layer="94">Date:</text>
<text x="12.7" y="1.27" size="2.54" layer="94">&gt;LAST_DATE_TIME</text>
<text x="72.39" y="1.27" size="2.54" layer="94">Sheet:</text>
<text x="86.36" y="1.27" size="2.54" layer="94">&gt;SHEET</text>
<text x="88.9" y="11.43" size="2.54" layer="94">REV:</text>
<text x="1.27" y="19.05" size="2.54" layer="94">TITLE:</text>
<text x="1.27" y="11.43" size="2.54" layer="94">Document Number:</text>
<text x="17.78" y="19.05" size="2.54" layer="94">&gt;DRAWING_NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="DINA4_L" prefix="FRAME" uservalue="yes">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A4, landscape with extra doc field</description>
<gates>
<gate name="G$1" symbol="DINA4_L" x="0" y="0"/>
<gate name="G$2" symbol="DOCFIELD" x="162.56" y="0" addlevel="must"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="F1" library="e-sicherungen" deviceset="FEHLER-STROM-SCHUTZSCHALTER_4-POL" device="" value="RCD Type B"/>
<part name="K1" library="e-schuetze-relais" deviceset="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL" device=""/>
<part name="K2" library="e-schuetze-relais" deviceset="HILFSSCHUETZ_13-14_23_24_33-34_43-44_OHNE_KONTAKTSPIEGEL" device=""/>
<part name="G1" library="e-stromversorgungselemente" deviceset="NETZTEIL_GLEICHSPANNUNG" device="" value="5V DC"/>
<part name="P1" library="e-messinstrumente" deviceset="ENERGIEZAEHLER/WATTSTUNDENZAEHLER" device=""/>
<part name="ESP1" library="e-klemmen" deviceset="EINSPEISUNG_3-N-PE" device=""/>
<part name="F2" library="e-sicherungen" deviceset="LEITUNGSSCHUTZSCHALTER" device="" value="10A"/>
<part name="U$1" library="ESP32-DEVKITV1" deviceset="ESP32DEVKITV1" device=""/>
<part name="REL2" library="App Store" deviceset="RELAY-1-SRD#RC" device="" value=""/>
<part name="REL1" library="App Store" deviceset="RELAY-1-SRD#RC" device="" value=""/>
<part name="U$5" library="EVSEWB" deviceset="EVSEWB" device=""/>
<part name="IEC62196" library="con-amp" deviceset="MTA07-156" device=""/>
<part name="FRAME1" library="frames" deviceset="DINA4_L" device=""/>
</parts>
<sheets>
<sheet>
<description>Simple-EVSE-ESP32 with Phase Switching</description>
<plain>
<text x="177.8" y="45.72" size="1.778" layer="91">Simple EVSE Wallbox with Phase Switching</text>
</plain>
<instances>
<instance part="F1" gate="G$1" x="48.26" y="157.48"/>
<instance part="K1" gate="G$1" x="109.22" y="152.4"/>
<instance part="K2" gate="G$1" x="144.78" y="152.4"/>
<instance part="G1" gate="G$1" x="76.2" y="86.36" rot="R270"/>
<instance part="P1" gate="G$1" x="96.52" y="152.4"/>
<instance part="ESP1" gate="G$1" x="48.26" y="60.96"/>
<instance part="F2" gate="G$1" x="76.2" y="152.4"/>
<instance part="U$1" gate="G$1" x="185.42" y="81.28" rot="R180"/>
<instance part="REL2" gate="G$1" x="139.7" y="86.36" rot="R270"/>
<instance part="REL1" gate="G$1" x="231.14" y="86.36" rot="R270"/>
<instance part="U$5" gate="G$1" x="215.9" y="149.86" rot="R90"/>
<instance part="IEC62196" gate="G$1" x="259.08" y="167.64" rot="R270"/>
<instance part="FRAME1" gate="G$1" x="12.7" y="17.78"/>
<instance part="FRAME1" gate="G$2" x="175.26" y="17.78"/>
</instances>
<busses>
</busses>
<nets>
<net name="L1-EXT" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="2"/>
<pinref part="ESP1" gate="G$1" pin="L1-EXT"/>
<wire x1="38.1" y1="144.78" x2="38.1" y2="68.58" width="0.1524" layer="91"/>
</segment>
</net>
<net name="L2-EXT" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="4"/>
<pinref part="ESP1" gate="G$1" pin="L2-EXT"/>
<wire x1="43.18" y1="144.78" x2="43.18" y2="68.58" width="0.1524" layer="91"/>
</segment>
</net>
<net name="L3-EXT" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="6"/>
<pinref part="ESP1" gate="G$1" pin="L3-EXT"/>
<wire x1="48.26" y1="144.78" x2="48.26" y2="68.58" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N-EXT" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="N'"/>
<pinref part="ESP1" gate="G$1" pin="N-EXT"/>
<wire x1="53.34" y1="144.78" x2="53.34" y2="68.58" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="1"/>
<wire x1="38.1" y1="162.56" x2="38.1" y2="193.04" width="0.1524" layer="91"/>
<wire x1="96.52" y1="193.04" x2="78.74" y2="193.04" width="0.1524" layer="91"/>
<wire x1="78.74" y1="193.04" x2="38.1" y2="193.04" width="0.1524" layer="91"/>
<pinref part="F2" gate="G$1" pin="1"/>
<wire x1="78.74" y1="157.48" x2="78.74" y2="193.04" width="0.1524" layer="91"/>
<junction x="78.74" y="193.04"/>
<pinref part="P1" gate="G$1" pin="I1"/>
<wire x1="96.52" y1="193.04" x2="96.52" y2="157.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="3"/>
<wire x1="43.18" y1="162.56" x2="43.18" y2="187.96" width="0.1524" layer="91"/>
<pinref part="K1" gate="G$1" pin="33"/>
<wire x1="43.18" y1="187.96" x2="127" y2="187.96" width="0.1524" layer="91"/>
<wire x1="127" y1="187.96" x2="127" y2="157.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="5"/>
<wire x1="48.26" y1="162.56" x2="48.26" y2="182.88" width="0.1524" layer="91"/>
<pinref part="K1" gate="G$1" pin="23"/>
<wire x1="48.26" y1="182.88" x2="121.92" y2="182.88" width="0.1524" layer="91"/>
<wire x1="121.92" y1="182.88" x2="121.92" y2="157.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$1" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="N"/>
<wire x1="53.34" y1="162.56" x2="53.34" y2="177.8" width="0.1524" layer="91"/>
<pinref part="K1" gate="G$1" pin="13"/>
<wire x1="53.34" y1="177.8" x2="73.66" y2="177.8" width="0.1524" layer="91"/>
<wire x1="73.66" y1="177.8" x2="109.22" y2="177.8" width="0.1524" layer="91"/>
<wire x1="109.22" y1="177.8" x2="116.84" y2="177.8" width="0.1524" layer="91"/>
<wire x1="116.84" y1="177.8" x2="116.84" y2="157.48" width="0.1524" layer="91"/>
<pinref part="K1" gate="G$1" pin="A1"/>
<wire x1="109.22" y1="177.8" x2="109.22" y2="157.48" width="0.1524" layer="91"/>
<pinref part="K2" gate="G$1" pin="A1"/>
<wire x1="116.84" y1="177.8" x2="144.78" y2="177.8" width="0.1524" layer="91"/>
<wire x1="144.78" y1="177.8" x2="144.78" y2="157.48" width="0.1524" layer="91"/>
<junction x="109.22" y="177.8"/>
<junction x="116.84" y="177.8"/>
<pinref part="U$5" gate="G$1" pin="N"/>
<wire x1="144.78" y1="177.8" x2="218.44" y2="177.8" width="0.1524" layer="91"/>
<wire x1="218.44" y1="177.8" x2="218.44" y2="165.1" width="0.1524" layer="91"/>
<junction x="144.78" y="177.8"/>
<pinref part="G1" gate="G$1" pin="2"/>
<wire x1="73.66" y1="96.52" x2="73.66" y2="177.8" width="0.1524" layer="91"/>
<junction x="73.66" y="177.8"/>
</segment>
</net>
<net name="N$5" class="0">
<segment>
<pinref part="K1" gate="G$1" pin="34"/>
<wire x1="127" y1="147.32" x2="127" y2="137.16" width="0.1524" layer="91"/>
<wire x1="127" y1="137.16" x2="152.4" y2="137.16" width="0.1524" layer="91"/>
<pinref part="K2" gate="G$1" pin="14"/>
<wire x1="152.4" y1="137.16" x2="152.4" y2="147.32" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<pinref part="K1" gate="G$1" pin="24"/>
<wire x1="121.92" y1="147.32" x2="121.92" y2="132.08" width="0.1524" layer="91"/>
<wire x1="121.92" y1="132.08" x2="157.48" y2="132.08" width="0.1524" layer="91"/>
<pinref part="K2" gate="G$1" pin="24"/>
<wire x1="157.48" y1="132.08" x2="157.48" y2="147.32" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$7" class="0">
<segment>
<pinref part="P1" gate="G$1" pin="U1"/>
<pinref part="K1" gate="G$1" pin="43"/>
<wire x1="132.08" y1="172.72" x2="132.08" y2="157.48" width="0.1524" layer="91"/>
<wire x1="132.08" y1="172.72" x2="99.06" y2="172.72" width="0.1524" layer="91"/>
<wire x1="99.06" y1="172.72" x2="99.06" y2="157.48" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<pinref part="G1" gate="G$1" pin="4"/>
<wire x1="134.62" y1="55.88" x2="73.66" y2="55.88" width="0.1524" layer="91"/>
<wire x1="73.66" y1="55.88" x2="73.66" y2="76.2" width="0.1524" layer="91"/>
<pinref part="REL1" gate="G$1" pin="VCC.1"/>
<wire x1="226.06" y1="63.5" x2="226.06" y2="55.88" width="0.1524" layer="91"/>
<wire x1="134.62" y1="55.88" x2="208.28" y2="55.88" width="0.1524" layer="91"/>
<wire x1="208.28" y1="55.88" x2="226.06" y2="55.88" width="0.1524" layer="91"/>
<wire x1="134.62" y1="55.88" x2="134.62" y2="63.5" width="0.1524" layer="91"/>
<pinref part="REL2" gate="G$1" pin="VCC.1"/>
<junction x="134.62" y="55.88"/>
<pinref part="U$1" gate="G$1" pin="VIN"/>
<wire x1="208.28" y1="63.5" x2="208.28" y2="55.88" width="0.1524" layer="91"/>
<junction x="208.28" y="55.88"/>
</segment>
</net>
<net name="N$17" class="0">
<segment>
<wire x1="185.42" y1="60.96" x2="137.16" y2="60.96" width="0.1524" layer="91"/>
<pinref part="REL2" gate="G$1" pin="IN1"/>
<wire x1="137.16" y1="60.96" x2="137.16" y2="63.5" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="IO32"/>
<wire x1="185.42" y1="63.5" x2="185.42" y2="60.96" width="0.1524" layer="91"/>
</segment>
</net>
<net name="L1" class="0">
<segment>
<pinref part="K1" gate="G$1" pin="44"/>
<wire x1="132.08" y1="147.32" x2="132.08" y2="142.24" width="0.1524" layer="91"/>
<wire x1="132.08" y1="142.24" x2="137.16" y2="142.24" width="0.1524" layer="91"/>
<wire x1="137.16" y1="142.24" x2="137.16" y2="193.04" width="0.1524" layer="91"/>
<wire x1="137.16" y1="193.04" x2="238.76" y2="193.04" width="0.1524" layer="91"/>
<wire x1="238.76" y1="193.04" x2="238.76" y2="167.64" width="0.1524" layer="91"/>
<wire x1="238.76" y1="167.64" x2="256.54" y2="167.64" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="1"/>
<label x="254" y="167.64" size="1.778" layer="95"/>
</segment>
</net>
<net name="N" class="0">
<segment>
<pinref part="K1" gate="G$1" pin="14"/>
<wire x1="116.84" y1="147.32" x2="116.84" y2="127" width="0.1524" layer="91"/>
<wire x1="116.84" y1="127" x2="233.68" y2="127" width="0.1524" layer="91"/>
<wire x1="233.68" y1="127" x2="233.68" y2="160.02" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="4"/>
<wire x1="233.68" y1="160.02" x2="256.54" y2="160.02" width="0.1524" layer="91"/>
<label x="254" y="160.02" size="1.778" layer="95"/>
</segment>
</net>
<net name="L2" class="0">
<segment>
<pinref part="K2" gate="G$1" pin="13"/>
<wire x1="152.4" y1="157.48" x2="152.4" y2="190.5" width="0.1524" layer="91"/>
<wire x1="152.4" y1="190.5" x2="236.22" y2="190.5" width="0.1524" layer="91"/>
<wire x1="236.22" y1="190.5" x2="236.22" y2="165.1" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="2"/>
<wire x1="236.22" y1="165.1" x2="256.54" y2="165.1" width="0.1524" layer="91"/>
<label x="254" y="165.1" size="1.778" layer="95"/>
</segment>
</net>
<net name="L3" class="0">
<segment>
<pinref part="K2" gate="G$1" pin="23"/>
<wire x1="157.48" y1="157.48" x2="157.48" y2="187.96" width="0.1524" layer="91"/>
<wire x1="157.48" y1="187.96" x2="233.68" y2="187.96" width="0.1524" layer="91"/>
<wire x1="233.68" y1="187.96" x2="233.68" y2="162.56" width="0.1524" layer="91"/>
<wire x1="233.68" y1="162.56" x2="256.54" y2="162.56" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="3"/>
<label x="254" y="162.56" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$18" class="0">
<segment>
<pinref part="K2" gate="G$1" pin="A2"/>
<wire x1="144.78" y1="147.32" x2="144.78" y2="109.22" width="0.1524" layer="91"/>
<pinref part="REL2" gate="G$1" pin="REL.1.NO"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<pinref part="K1" gate="G$1" pin="A2"/>
<wire x1="109.22" y1="147.32" x2="109.22" y2="124.46" width="0.1524" layer="91"/>
<wire x1="109.22" y1="124.46" x2="170.18" y2="124.46" width="0.1524" layer="91"/>
<wire x1="170.18" y1="124.46" x2="170.18" y2="167.64" width="0.1524" layer="91"/>
<pinref part="U$5" gate="G$1" pin="REL"/>
<wire x1="208.28" y1="165.1" x2="208.28" y2="167.64" width="0.1524" layer="91"/>
<wire x1="208.28" y1="167.64" x2="170.18" y2="167.64" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$20" class="0">
<segment>
<pinref part="U$5" gate="G$1" pin="RXD"/>
<wire x1="208.28" y1="134.62" x2="208.28" y2="116.84" width="0.1524" layer="91"/>
<wire x1="208.28" y1="116.84" x2="182.88" y2="116.84" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="IO21"/>
<wire x1="182.88" y1="99.06" x2="182.88" y2="116.84" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$21" class="0">
<segment>
<pinref part="U$5" gate="G$1" pin="TXD"/>
<wire x1="175.26" y1="99.06" x2="175.26" y2="119.38" width="0.1524" layer="91"/>
<wire x1="175.26" y1="119.38" x2="205.74" y2="119.38" width="0.1524" layer="91"/>
<wire x1="205.74" y1="119.38" x2="205.74" y2="134.62" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="IO22"/>
</segment>
</net>
<net name="N$19" class="0">
<segment>
<pinref part="U$5" gate="G$1" pin="CP"/>
<pinref part="REL1" gate="G$1" pin="REL.1.NC"/>
<wire x1="226.06" y1="134.62" x2="226.06" y2="109.22" width="0.1524" layer="91"/>
</segment>
</net>
<net name="CP" class="0">
<segment>
<pinref part="REL1" gate="G$1" pin="REL.1.COM"/>
<wire x1="231.14" y1="109.22" x2="231.14" y2="111.76" width="0.1524" layer="91"/>
<wire x1="231.14" y1="111.76" x2="241.3" y2="111.76" width="0.1524" layer="91"/>
<wire x1="241.3" y1="111.76" x2="241.3" y2="152.4" width="0.1524" layer="91"/>
<wire x1="241.3" y1="152.4" x2="256.54" y2="152.4" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="7"/>
<wire x1="256.54" y1="152.4" x2="259.08" y2="152.4" width="0.1524" layer="91"/>
<label x="254" y="152.4" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$23" class="0">
<segment>
<pinref part="P1" gate="G$1" pin="U2"/>
<wire x1="99.06" y1="144.78" x2="99.06" y2="111.76" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="IO16"/>
<wire x1="195.58" y1="99.06" x2="195.58" y2="111.76" width="0.1524" layer="91"/>
<wire x1="195.58" y1="111.76" x2="99.06" y2="111.76" width="0.1524" layer="91"/>
</segment>
</net>
<net name="PP" class="0">
<segment>
<pinref part="U$5" gate="G$1" pin="PP"/>
<wire x1="223.52" y1="134.62" x2="223.52" y2="116.84" width="0.1524" layer="91"/>
<wire x1="223.52" y1="116.84" x2="238.76" y2="116.84" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="6"/>
<wire x1="256.54" y1="154.94" x2="238.76" y2="154.94" width="0.1524" layer="91"/>
<wire x1="238.76" y1="154.94" x2="238.76" y2="116.84" width="0.1524" layer="91"/>
<label x="254" y="154.94" size="1.778" layer="95"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<wire x1="198.12" y1="104.14" x2="215.9" y2="104.14" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="IO4"/>
<wire x1="198.12" y1="99.06" x2="198.12" y2="104.14" width="0.1524" layer="91"/>
<wire x1="215.9" y1="60.96" x2="228.6" y2="60.96" width="0.1524" layer="91"/>
<wire x1="228.6" y1="60.96" x2="228.6" y2="63.5" width="0.1524" layer="91"/>
<wire x1="215.9" y1="104.14" x2="215.9" y2="60.96" width="0.1524" layer="91"/>
<pinref part="REL1" gate="G$1" pin="IN1"/>
</segment>
</net>
<net name="PE-EXT" class="0">
<segment>
<pinref part="ESP1" gate="G$1" pin="PE-EXT"/>
<wire x1="58.42" y1="68.58" x2="58.42" y2="121.92" width="0.1524" layer="91"/>
<wire x1="175.26" y1="121.92" x2="175.26" y2="172.72" width="0.1524" layer="91"/>
<pinref part="U$5" gate="G$1" pin="PE"/>
<wire x1="175.26" y1="172.72" x2="213.36" y2="172.72" width="0.1524" layer="91"/>
<wire x1="213.36" y1="172.72" x2="213.36" y2="165.1" width="0.1524" layer="91"/>
<wire x1="175.26" y1="121.92" x2="218.44" y2="121.92" width="0.1524" layer="91"/>
<pinref part="IEC62196" gate="G$1" pin="5"/>
<wire x1="218.44" y1="121.92" x2="236.22" y2="121.92" width="0.1524" layer="91"/>
<wire x1="256.54" y1="157.48" x2="236.22" y2="157.48" width="0.1524" layer="91"/>
<wire x1="236.22" y1="157.48" x2="236.22" y2="121.92" width="0.1524" layer="91"/>
<label x="254" y="157.48" size="1.778" layer="95"/>
<wire x1="175.26" y1="121.92" x2="96.52" y2="121.92" width="0.1524" layer="91"/>
<junction x="175.26" y="121.92"/>
<pinref part="U$5" gate="G$1" pin="GND"/>
<wire x1="96.52" y1="121.92" x2="58.42" y2="121.92" width="0.1524" layer="91"/>
<wire x1="218.44" y1="134.62" x2="218.44" y2="121.92" width="0.1524" layer="91"/>
<junction x="218.44" y="121.92"/>
<pinref part="P1" gate="G$1" pin="I2"/>
<pinref part="REL2" gate="G$1" pin="GND"/>
<pinref part="G1" gate="G$1" pin="3"/>
<wire x1="78.74" y1="76.2" x2="78.74" y2="58.42" width="0.1524" layer="91"/>
<wire x1="139.7" y1="58.42" x2="96.52" y2="58.42" width="0.1524" layer="91"/>
<wire x1="96.52" y1="58.42" x2="78.74" y2="58.42" width="0.1524" layer="91"/>
<wire x1="139.7" y1="58.42" x2="139.7" y2="63.5" width="0.1524" layer="91"/>
<junction x="139.7" y="58.42"/>
<pinref part="REL1" gate="G$1" pin="GND"/>
<wire x1="231.14" y1="58.42" x2="231.14" y2="63.5" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="GND1"/>
<wire x1="205.74" y1="63.5" x2="205.74" y2="58.42" width="0.1524" layer="91"/>
<wire x1="205.74" y1="58.42" x2="231.14" y2="58.42" width="0.1524" layer="91"/>
<wire x1="205.74" y1="58.42" x2="139.7" y2="58.42" width="0.1524" layer="91"/>
<junction x="205.74" y="58.42"/>
<wire x1="96.52" y1="144.78" x2="96.52" y2="121.92" width="0.1524" layer="91"/>
<junction x="96.52" y="58.42"/>
<junction x="96.52" y="121.92"/>
<wire x1="96.52" y1="121.92" x2="96.52" y2="58.42" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<pinref part="REL2" gate="G$1" pin="REL.1.COM"/>
<pinref part="F2" gate="G$1" pin="2"/>
<wire x1="78.74" y1="147.32" x2="78.74" y2="119.38" width="0.1524" layer="91"/>
<wire x1="78.74" y1="119.38" x2="139.7" y2="119.38" width="0.1524" layer="91"/>
<wire x1="139.7" y1="119.38" x2="139.7" y2="109.22" width="0.1524" layer="91"/>
<wire x1="139.7" y1="119.38" x2="172.72" y2="119.38" width="0.1524" layer="91"/>
<wire x1="172.72" y1="119.38" x2="172.72" y2="170.18" width="0.1524" layer="91"/>
<pinref part="U$5" gate="G$1" pin="L"/>
<wire x1="223.52" y1="170.18" x2="223.52" y2="165.1" width="0.1524" layer="91"/>
<wire x1="172.72" y1="170.18" x2="223.52" y2="170.18" width="0.1524" layer="91"/>
<junction x="139.7" y="119.38"/>
<wire x1="78.74" y1="119.38" x2="78.74" y2="96.52" width="0.1524" layer="91"/>
<pinref part="G1" gate="G$1" pin="1"/>
<junction x="78.74" y="119.38"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
