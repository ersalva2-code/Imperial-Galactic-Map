using System;
using System.Collections.Generic;
using System.Linq;
using System.Drawing;
using System.Drawing.Drawing2D;
namespace IGM {
public static class ImperialCore {
 public const string ID="275889132_1121173938_1505499062_4230148680";
 static readonly string[][] rows={
  new[]{"0000010011111110","","Imperio Español Centro reclutamiento IVX"},
  new[]{"00000200f0cd27ab","","Estación Tau Exmout-Yedaw"},
  new[]{"00000b00a37323b6","Territorio imperial español / Sis. Mineria","Estación Sigma Raitap"},
  new[]{"00000d0901210272","Hispalis / Territorio Imperial Español","Imperio español Estacion España IX"},
  new[]{"00001300a37323b6","Ioviis XVI  / Territorio Imperial Español","Ioviis XVI Station Sigma"},
  new[]{"00001300f9383a22","Fumaxgazy","Intercambio Fumaxgazy"},
  new[]{"0000160925753234","Velez / Territorio Imperial Español","Centro de comercio España VI"},
  new[]{"00001800a37323b6","Olufils / Territorio imperial espanol 1403","Estación Sigma Olufils"},
  new[]{"000019000138a5ee","Astra Espousal","Astra Espousal Prime"},
  new[]{"0000200010010011","","Reclutamiento imperial español"},
  new[]{"00002100a47323b6","Rizas-Tamba / Territorio Imperial Español","Rizonas-Tamba V Station"},
  new[]{"00002200a37323b6","Erberg Territorio imperial español 200","Territorio imperial español 200"},
  new[]{"00002200a47323b6","Fordwi-Kal / Territorio Imperial Español","Fordwi-Kals XVIII Station"},
  new[]{"00002300a37323b6","Orfattak / Territorio Imperial Español","Orfattak Station Sigma"},
  new[]{"00002400a37323b6","Comple-Sek / Territorio Imperial Español","Comple-Sek Station Sigma"},
  new[]{"00002b00a37323b6","","Harafhen Station Sigma"},
  new[]{"00002c00a37323b6","Ruttinifal  / Territorio Imperial Español","Ruttinifal XIII Station Sigma"},
  new[]{"00002d00a37323b6","Evenin-Led / Territorio Imperial Español","Evenin-Led Station Sigma"},
  new[]{"00002e00a37323b6","Palmen-Umid / Territorio Imperial Español","Palmen-Umid Station Sigma"},
  new[]{"00003000a37323b6","Nenyos / TERRITORIO IMPERIAL ESPANOL  1404","Sigma / territorio imperial espanol 1043"},
  new[]{"00003a00f86801d6","Novkin / Territorio Imperial Español","Estación Novkin-Gifir XVI"},
  new[]{"00003f00a37323b7","Hizuishu / Territorio Imperial Español","Hizuishu Platform"},
  new[]{"0000430902301320","Marbella / Territorio Imperial Español","Centro España V"},
  new[]{"00004400a37323b6","Capital Imperial española","imperio espanol Estacion capital imperial"},
  new[]{"00004409239d8bef","España / Territorio Imperial Español","Blas de Lezo"},
  new[]{"00004500a37323b6","Eftirl / Territorio Imperial Español","Eftirl Station Sigma"},
  new[]{"00004600a37323b6","Ohandlin / Territorio Imperial Español","Ohandlin Station Sigma"},
  new[]{"00004e09026fd0a8","","Estación alfa Yortlei"},
  new[]{"00005200a37323b6","Territorio imperial español - Mineria II","Estación Sigma Bolgang"},
  new[]{"00005200fb004005","","Sistema central español Estacion XX"},
  new[]{"0000560914572054","Sistema Solar V","Centro España VII"},
  new[]{"00005800a47323b6","Rementn / Territorio imperial Espanol","Estación Rementn-Oleid"},
  new[]{"00005c00a37323b6","Orygog Territorio imperial español 201","Territorio imperial español 201"},
  new[]{"00006000a37323b6","Loekre-Lulo /Territorio imperial Espanol","Estación Sigma Loekre-Lulo"},
  new[]{"00006100a37323b6","Vatnin XVII / Territorio Imperial Español","Vatnin XVII Station Sigma"},
  new[]{"00006500f9234983","Yakodaita / Territorio Imperial Español","Término de Yakodaita III"},
  new[]{"00006700a37323b6","Persat-Idor / Territorio Imperial Español","Persat-Idor Station Sigma"},
  new[]{"00006700a47323b6","Edmouti-Kuc / Territorio Imperial Español","Edmouti-Kuc Station"},
  new[]{"00006900a37323b6","","Nisblad IX Station Sigma"},
  new[]{"00006a00a37323b6","Amuuto-Meif / Territorio Imperial Español","Amuuto-Meif Station Sigma"},
  new[]{"00006c00a37323b6","Voinei-Kan / Territorio Imperial Español","Voinei-Kan Station Sigma"},
  new[]{"00006e00a37323b6","Tatamild X  / Territorio Imperial Español","Tatamild XIII Station Sigma"},
  new[]{"00006f00a37323b6","Igognupra / Territorio Imperial Español","Igognupra Station Sigma"},
  new[]{"00007200a37323b6","Pusatta / Territorio imperial Espanol","Estación Sigma Pusatta"},
  new[]{"0000720906475702","Altorre / Territorio Imperial Español","Imperio Español España VIII"},
  new[]{"00007a00a37323b7","Genning / Territorio Imperial Español","Genning Platform"},
  new[]{"00008700a37323b7","Ilhunco / Territorio Imperial Español","Ilhunco Platform"},
  new[]{"00009f00fb004005","","centro espanol XXV"},
  new[]{"0000a400fb003006","","Sistema central español Estacion XXII"},
  new[]{"0000a600fa002005","Mr. Brownie","Sistema central español Estacion XXIV"},
  new[]{"0000b400f9007008","","Imperio Español estacion reclutamiento V"},
  new[]{"0000b600a37323b7","","Uiyang-Azd Platform"},
  new[]{"0000b600fb003006","","Sistema central español Estacion XXI"},
  new[]{"0000c500fb004005","","Sistema central español Estacion XVII"},
  new[]{"0000c600a37323b7","Daejeo  / Territorio Imperial Español","Daejeo Platform"},
  new[]{"0000c900a37323b7","Luluda / Territorio Imperial Español","Luluda Platform"},
  new[]{"0000c900fb004005","","Sistema central Español Estacion XVI"},
  new[]{"0000cb00a37323b7","Iryari  / Territorio Imperial Español","Iryari Platform"},
  new[]{"0000cb0911111102","Hispania / Territorio Imperial Español","Estacion Imperial Española X"},
  new[]{"0000cf00fb004005","","Estación Drezov"},
  new[]{"0000d400fb002006","","Sistema central español Estacion XXIII"},
  new[]{"0000e400fb004005","47_MidEco_Vaikin","Imperio Español etacion reclutaminto XIII"},
  new[]{"0000fb0008dcf671","Rishio III","La esfera Rishio III"},
  new[]{"0000fe00fb005006","","Imperio Español estacion reclutamiento X"},
  new[]{"00010100fb004005","","Estación Hukingi-Mei"},
  new[]{"00010600fa006007","","Imperio Español estacion reclutamiento VII"},
  new[]{"0001120011112022","","Orbital Shogan-Epets"},
  new[]{"00011e00fb004005","","Imperio Español estacion reclutamiento XII"},
  new[]{"00012100f8008008","","Imperio Español Centro reclutamiento IV"},
  new[]{"000122140322010e","Hispania / Territorio Imperial Español","Centro comercio IMPERIO ESPAñOL"},
  new[]{"00012800f76ad0e9","Roishin / Territorio Imperial Español","Término de Roishin II"},
  new[]{"00013500fb005006","","Imperio Español estacion reclutamiento IX"},
  new[]{"00014500fb004005","","Sistema central español Estacion XVIII"},
  new[]{"000155140222010f","Lusitania / Territorio Imperial Español","Centro de comercio Español"},
  new[]{"00017000f893efb8","","Plataforma Isgarati XV"},
  new[]{"00019700a37323b6","Yoismidi / Territorio Imperial Español","Yoismidi Station Sigma"},
  new[]{"0001b000fb004005","","Imperio Español estacion reclutamiento XI"},
  new[]{"0001be0903003ff9","","Imperio Español Centro de reclutamiento II"},
  new[]{"0001d00903003ff9","","Imperio Español Centro de reclutamiento II"},
  new[]{"0001db00003b2a74","Sistema Refinero Industrial","Sistema Refinero Industrial Cycler"},
  new[]{"0001e600fa006006","","Imprio Español estacion reclutamiento VIII"},
  new[]{"0001e70903003ff9","","Imoperio Español Centro reclutamiento III"},
  new[]{"0001ec0903003ff9","","Imperio Español Centro de reclutamiento"},
  new[]{"0001ed0901210212","Cazorla / Territorio Imperial Español","Comercio Español IV"},
  new[]{"0001f700fb004005","","Sistema central español Estacion IX"},
  new[]{"0001f800a37323b6","Fritel-Lag X / Territorio Imperial Español","Fritel-Lag X Station Sigma"},
  new[]{"0001fa00f9007007","","Imperio Español estacion reclutamiento VI"},
  new[]{"000205000094a219","","Regoind-Usuka Station Major"},
  new[]{"00021100f808f6d2","","Ciclovía Yumenn"},
  new[]{"0003f000a37323b6","Elhelug-Ok / Territorio Imperial Español","Elhelug-Okut Station Sigma"},
  new[]{"00040e00a37323b6","Ugarfriti / Territorio Imperial Español","Ugarfriti Station Sigma"},
  new[]{"00042100a3732635","Triceanico / Territorio Imperial Español","Base imperial Triceanica"},
 };
 public static int Count {get{return rows.Length;}}
 public static bool IsAlliance(string id){return id==ID;}
 public static bool Contains(string ua){return rows.Any(r=>string.Equals(r[0],ua,StringComparison.OrdinalIgnoreCase));}
 public static bool Locked(SystemRecord r){return Contains(r.UA)||IsAlliance(r.Alliance);}
 public static void Apply(Project p){
 var a=p.Alliances.Find(z=>z.ID==ID);if(a==null){a=new Alliance{ID=ID};p.Alliances.Insert(0,a);}else{p.Alliances.Remove(a);p.Alliances.Insert(0,a);}
 a.Name="Imperio Español";a.Initials="IESP";a.ColorRGB=Color.White.ToArgb();a.NameEdited=false;a.Icon="";a.PreviewPng="";a.ImagePng="";a.SymbolPath="";a.Opacity=100;a.UseDiamond=true;
 foreach(var row in rows){var r=p.Systems.Find(z=>string.Equals(z.UA,row[0],StringComparison.OrdinalIgnoreCase)&&z.Planet==0);if(r==null){r=new SystemRecord{UA=row[0]};p.Systems.Add(r);}r.UA=row[0];r.Alliance=ID;r.Name=row[1];r.Station=row[2];r.Selected=true;r.Evidence="protected_imperial_core";}
 }
 public static void DrawBadge(Graphics g,Rectangle r){
 var save=g.Save();g.TranslateTransform(r.X+r.Width/2f,r.Y+r.Height/2f);g.ScaleTransform(r.Width/26.4f,r.Height/26.4f);g.SmoothingMode=SmoothingMode.AntiAlias;
 using(var b=new SolidBrush(Color.FromArgb(64,64,64)))g.FillPolygon(b,new[]{new PointF(0,-13.2f),new PointF(13.2f,0),new PointF(0,13.2f),new PointF(-13.2f,0)});
 using(var b=new SolidBrush(Color.White))g.FillPolygon(b,new[]{new PointF(0,-11.6f),new PointF(11.6f,0),new PointF(0,11.6f),new PointF(-11.6f,0)});
 using(var b=new SolidBrush(Color.FromArgb(196,20,43))){for(int diagonal=-1;diagonal<=1;diagonal+=2){Plank(g,b,-5.4f,-5.4f*diagonal,5.4f,5.4f*diagonal,2.3f);for(int t=-3;t<=3;t+=3)for(int side=-1;side<=1;side+=2)Plank(g,b,t,t*diagonal,t+side*1.8f,t*diagonal-side*diagonal*1.8f,1.4f);}}
 g.Restore(save);
 }
 static void Plank(Graphics g,Brush b,float x1,float y1,float x2,float y2,float width){float dx=x2-x1,dy=y2-y1,len=(float)Math.Sqrt(dx*dx+dy*dy),nx=-dy/len*width/2,ny=dx/len*width/2;var points=new[]{new PointF(x1+nx,y1+ny),new PointF(x2+nx,y2+ny),new PointF(x2-nx,y2-ny),new PointF(x1-nx,y1-ny)};for(int i=0;i<points.Length;i++){float span=Math.Abs(points[i].X)+Math.Abs(points[i].Y);if(span>10.9f){points[i].X*=10.9f/span;points[i].Y*=10.9f/span;}}g.FillPolygon(b,points);}
} }
