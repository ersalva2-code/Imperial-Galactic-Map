# DLL Steam 0.55
Windows x64, Zig cc. Desde la raíz de las fuentes:

```sh
zig cc -target x86_64-windows-gnu -shared -O2 -s -Wall -Wextra -Werror -Wno-cast-function-type -I FUENTE/minhook/include -I FUENTE/minhook/src FUENTE/igm_reader.c FUENTE/minhook/src/hook.c FUENTE/minhook/src/buffer.c FUENTE/minhook/src/trampoline.c FUENTE/minhook/src/hde/hde64.c -ladvapi32 -luser32 -o xinput9_1_0.dll
```

Helper WGC: se distribuye sin cambios respecto a 0.50, SHA256
bdd7455da716bc917fe1bf3fcd637eb230822f50e2693aef9b90c2557731fd2e.
Su fuente es igm_capture50.c y wgc50_abi.h. Las rutas se pasan por argumentos.

Companion 0.22: compilar Core.cs + ImperialCore.cs + Interface.cs como winexe x64 contra las
referencias de .NET Framework 4.8, System, System.Core, System.Drawing,
System.Windows.Forms y System.Web.Extensions; incluir glyphs.png como recurso
IGM.Glyphs y usar IGM_Companion.ico. No compilar contra las implementaciones
Mono: sus sobrecargas no son idénticas a .NET Framework de Windows.

Los test_*.c se compilan con GCC C11 y -lm. test_compat y test_menu_scan
reciben la ruta del EXE como argumento. test_layout51 requiere -D_GNU_SOURCE
-include unistd.h en Linux. Verify020.cs se compila junto a Core.cs y recibe
las rutas de partidas .hg. No incluir Verify020.cs en el ejecutable gráfico.
Las pruebas de API nativa usan stubs, no ejecutan NMS ni su GPU.

## Emblemas 0.52
icon52_format.h valida BMP BGRA32 top-down 180x180 y decodifica IGMICON1/2.
icon52_data.inc carga los archivos al arrancar, antes de instalar los hooks.
icon52_render.inc comparte el backend validado de las fotos; una textura por
alianza, máximo 128, carga como máximo una por pasada. En el mapa hereda la
misma transformación, desplazamiento y desvanecimiento que los marcadores.
Los dibujos originales (incluida Borgoña) se mantienen como fallback.
Core.cs exporta BMP completo desde PreviewPng además del .igmicon histórico.
No compilar Verify*.cs dentro del programa gráfico.

test_icon52 y test_icon52_load reciben el BMP generado por Verify020 en
/tmp/companion52-roundtrip.bmp. test_icon52_load requiere -D_GNU_SOURCE
-include unistd.h en Linux. Estas pruebas usan el decodificador de producción
y el mismo archivo exportado por el Core, no una recreación en Python.

## Core imperial 0.53
86 sistemas compilados. known_state53.inc fuerza el ID oficial antes de las observaciones.
menu_data.inc ignora exclusiones antiguas y rechaza borrado/renombrado del core.
icon52_data.inc ignora cualquier emblema externo para el ID imperial.
ImperialCore.cs se aplica al inicio, al importar, abrir proyectos y antes de exportar.
El exportador omite registros y emblemas imperiales. La interfaz no ofrece editor para IESP.
VerifyImperial.cs sustituye Verify020.cs para las pruebas del nuevo comportamiento protegido.
Los test_imperial53*.c usan las funciones de produccion con APIs de OS simuladas.

## 0.54
position54_patterns.h se resuelve en compat_scan.h y se comprueba en codigo vivo.
position54.h implementa cache propia y presupuesto asincrono. position54_render.inc
comparte las funciones de posicion/proyeccion de produccion con las pruebas.
POSITION54_NOTES.md documenta ABI y limites. Companion y capturador sin cambios.

## 0.55
position54_render.inc restaura prioridad de anclas nativas y usa output.w de
la proyeccion nativa para las posiciones independientes. Ver POSITION55_NOTES.md.
