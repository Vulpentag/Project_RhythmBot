# 🎼 RhythmBot

**Bot de automatización experimental para juegos de ritmo en Windows.**

RhythmBot captura una región de la pantalla en tiempo real, detecta notas por color con OpenCV y simula las pulsaciones de teclado correspondientes a cada carril. Está diseñado como proyecto de aprendizaje e investigación sobre visión por computador, captura de pantalla en Windows y simulación de entrada de bajo nivel.

> ⚠️ **Aviso**: este proyecto es un prototipo experimental. Puede violar los Términos de Servicio de juegos online y activar sistemas anti-cheat. Úsalo solo en juegos offline, entornos controlados o con fines educativos. El autor no se responsabiliza del mal uso.

---

## 🧩 ¿Qué hace?

- Captura una región rectangular de la pantalla mediante **GDI (WinAPI)**.
- Convierte cada frame a **HSV** y detecta las notas por rango de color.
- Clasifica cada nota detectada en uno de los **cuatro carriles** según su posición X.
- Cuando una nota se aproxima a una **línea objetivo configurable**, simula la pulsación de la tecla asignada a ese carril.
- Incluye un **modo calibración** para ajustar visualmente la región de captura y muestrear el color de las notas.

---

## 🛠️ Tecnologías

| Componente | Detalle |
|---|---|
| Lenguaje | **C++17** |
| Compilador | **MSVC** (Visual Studio) |
| Build system | **CMake** ≥ 3.20 |
| Gestor de paquetes | **vcpkg** |
| Visión por computador | **OpenCV 4.x** (`core`, `imgproc`, `highgui`) |
| Captura de pantalla | **GDI / WinAPI** (`BitBlt`, `DIB section`) |
| Simulación de entrada | **`SendInput`** con scan codes y **`PostMessage`** |
| Plataforma | **Windows 10/11 (x64)** |

---

## ⚙️ Cómo funciona (resumen)

1. `ScreenCapture` captura la región configurada con un **DIB section reutilizable** para evitar reasignaciones por frame.
2. `NoteDetector` convierte el frame a HSV, aplica un filtro `inRange`, limpia la máscara con operaciones morfológicas y busca contornos externos. Los buffers y el kernel se reutilizan entre frames.
3. `Controller` toma las notas detectadas, elige por carril la más cercana a la línea objetivo y, cuando la distancia está dentro de la tolerancia configurada, envía la tecla. Antes de enviarla, comprueba que la ventana del juego esté en primer plano para evitar pulsaciones fuera de contexto.
4. `Calibrator` ofrece una vista ampliada de la captura con trackbars para ajustar la región, y permite muestrear el color de cualquier píxel haciendo clic sobre él. Los valores obtenidos deben copiarse manualmente a `Config.h`.

---

## 📁 Estructura del proyecto

```text
RhythmBot/
├── include/
│   ├── Config.h             # Parámetros del bot
│   ├── ScreenCapture.h      # Captura de pantalla (GDI)
│   ├── NoteDetector.h       # Detección de notas (OpenCV)
│   ├── KeySimulator.h       # Simulación de teclado
│   ├── Controller.h         # Lógica principal
│   └── Calibrator.h         # Modo calibración
├── src/
│   ├── ScreenCapture.cpp    # Implementación GDI
│   ├── NoteDetector.cpp     # Detección con OpenCV
│   ├── KeySimulator.cpp     # Envío de teclas (SendInput)
│   ├── Controller.cpp       # Bucle principal
│   ├── Calibrator.cpp       # Modo calibración
│   └── main.cpp             # Punto de entrada
├── CMakeLists.txt
├── .gitignore
└── README.md
```
---

## 🔧 Configuración actual

Los valores por defecto en `Config.h` son:

| Parámetro | Valor |
|---|---|
| Región de captura | `X=437, Y=500, 490×190 px` |
| Teclas por carril | `A`, `S`, `K`, `L` |
| Línea objetivo (`hitY`) | `145` (relativo a la región capturada) |
| Tolerancia (`tolY`) | `18 px` |
| Vista de depuración | Desactivada por defecto |

La lógica del bot **no pulsa al aparecer la nota**, sino cuando la nota **se aproxima a la línea objetivo**. Esto permite ajustar el punto de pulsación según la latencia del sistema.

---
