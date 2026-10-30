using System;using System.IO;using System.Linq;using System.Drawing;using System.Drawing.Imaging;using IGM;
class VerifyImperial {
 static void Check(bool ok,string what){if(!ok)throw new Exception(what);}
 static void Main(string[]args){
 var p=new Project();ImperialCore.Apply(p);Check(p.Systems.Count==92&&p.Alliances.Count==1,"standalone core");Check(p.Systems.Count(s=>s.Name.Length>0)==57,"confirmed names");
 string ua=p.Systems.First(s=>s.Name.Length>0).UA,canonical=p.Systems.First(s=>s.UA==ua).Name;
 p.Systems.RemoveAt(0);p.Systems.First(s=>s.UA==ua).Alliance="1_2_3_4";p.Systems.First(s=>s.UA==ua).Name="tampered";p.Alliances[0].Name="tampered";p.Alliances[0].PreviewPng="invalid";ImperialCore.Apply(p);
 Check(p.Systems.Count==92&&p.Alliances[0].Initials=="IESP"&&p.Alliances[0].Name=="Imperio Español","restore deleted core and identity");Check(p.Systems.First(s=>s.UA==ua).Name==canonical&&p.Systems.First(s=>s.UA==ua).Alliance==ImperialCore.ID,"restore core assignment/name");
 foreach(string file in args){var imported=Data.Import(file,"");Check(imported.Alliances.Any(z=>z.ID==ImperialCore.ID)&&imported.Systems.Count(r=>ImperialCore.Contains(r.UA))==92,"real save includes immutable core");Console.WriteLine("IMPORT actual Steam "+Path.GetFileName(file)+" own="+imported.Systems.Count(r=>r.Owned)+" total="+imported.Systems.Count);}
 string bundle=Path.Combine(Path.GetTempPath(),"IGM protected test "+Guid.NewGuid().ToString("N"),"IGM"),folder=Data.MigrateBundle(bundle);string dir=Path.Combine(folder,"iconos_alianzas");Directory.CreateDirectory(dir);
 string iconPath=Path.Combine(dir,ImperialCore.ID+".bmp"),oldIcon=Path.Combine(dir,ImperialCore.ID+".igmicon");File.WriteAllText(iconPath,"preserve historical imperial image");File.WriteAllText(oldIcon,"invalid historical icon ignored");
 string cat=Path.Combine(folder,"catalogo_observado.jsonl");string row=Data.Json().Serialize(new{system_ua=ua,alliance_id_words=new uint[]{1,2,3,4},cached_alliance_name="external hostile",source="test"})+"\n";File.WriteAllText(cat,row);
 string management=Path.Combine(folder,"gestion_sistemas.tsv"),names=Path.Combine(folder,"exploracion.tsv");File.WriteAllText(management,ua+"\t2\t0\t\n");File.WriteAllText(names,ua+"\t0\t-1\t1\t"+Data.Hex("tampered")+"\n");string oldNames=File.ReadAllText(names);
 var local=Data.LoadMod(folder);Check(local.Systems.First(s=>s.UA==ua).Alliance==ImperialCore.ID&&local.Systems.First(s=>s.UA==ua).Name==canonical,"local file cannot override compiled core");
 var only=new Project();ImperialCore.Apply(only);string result=Data.Export(only,folder,false);Check(result=="0 / conflicts: 0","core-only export is no-op");Check(File.ReadAllText(cat)==row&&File.ReadAllText(names)==oldNames&&File.ReadAllText(iconPath)=="preserve historical imperial image"&&File.ReadAllText(oldIcon)=="invalid historical icon ignored","export does not touch imperial legacy data");
 var a=new Alliance{ID="9_8_7_6",Name="User alliance",Initials="USR"};p.Alliances.Add(a);p.Systems.Add(new SystemRecord{UA="0000990012345678",Alliance=a.ID,Name="User system"});
 using(var image=new Bitmap(180,180)){using(var g=Graphics.FromImage(image))g.Clear(Color.FromArgb(255,119,51,17));image.SetPixel(1,0,Color.FromArgb(85,120,51,18));image.SetPixel(90,90,Color.Transparent);using(var stream=new MemoryStream()){image.Save(stream,ImageFormat.Png);a.PreviewPng=Convert.ToBase64String(stream.ToArray());}}
 result=Data.Export(p,folder,false);Check(result.StartsWith("1 / names: 1 / icons: 1 / conflicts: 0"),"user export functional");Check(File.ReadAllText(iconPath)=="preserve historical imperial image"&&File.ReadAllText(oldIcon)=="invalid historical icon ignored","user export preserves imperial files");Check(File.ReadAllText(cat).StartsWith(row)&&File.ReadAllLines(cat).Length==2,"append user without replacing catalogue");Check(File.ReadAllText(management)==ua+"\t2\t0\t\n","favourites unchanged");
 byte[] bmp=File.ReadAllBytes(Path.Combine(dir,a.ID+".bmp"));Check(bmp.Length==54+180*180*4&&bmp[54]==17&&bmp[55]==51&&bmp[56]==119,"custom image full colour/size");File.WriteAllBytes("/tmp/companion52-roundtrip.bmp",bmp);
 Check(Data.Export(p,folder,false)=="0 / conflicts: 0","idempotent export");
 string project=Path.Combine(bundle,"test.json");Data.SaveProject(p,project);var reopened=Data.OpenProject(project);Check(reopened.Systems.Count(s=>ImperialCore.Contains(s.UA))==92,"open project preserves compiled core");
 using(var img=new Bitmap(576,576))using(var g=Graphics.FromImage(img)){g.Clear(Color.Transparent);ImperialCore.DrawBadge(g,new Rectangle(0,0,576,576));img.Save("/tmp/imperial53-emblem.png",ImageFormat.Png);Check(img.GetPixel(288,288).R>150&&img.GetPixel(288,288).G<50,"original red Borgona cross");}
 Console.WriteLine("PASS .NET 4.8 core: 92 compiled records, 57 names, protected identity/emblem, malicious local overrides rejected, immutable export, user append/icons and repeat export, project reload, real Steam imports. No save writes.");
 }
}
