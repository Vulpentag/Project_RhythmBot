# RhythmBot v0.1.0-alpha

Primera versión pública del bot de ritmo para Windows. **Prototipo funcional** con detección de notas por visión por computador y simulación de teclado.

## 🎉 Novedades

### Detección
- Captura de pantalla en tiempo real vía GDI (WinAPI).
- Detección de notas por rango HSV con OpenCV.
- Clasificación automática de carriles por posición X.

### Simulación de teclado
- Envío con `SendInput` usando **scan codes** (mayor compatibilidad con juegos modernos).
- Envío paralelo con `PostMessage` directo a la ventana del juego.
- Anti-repetición: evita pulsar la misma nota múltiples veces.

### Calibrador
- Modo calibración con trackbars para ajustar la región de captura.
- Muestreo de color por clic en cualquier píxel de la pantalla.
- Vista previa ampliada ×2 con overlays de carriles.

### Rendimiento
- Optimizado para equipos modestos (probado en AMD Athlon 300U).
- Bucle configurable de 30 a 240 FPS.
- Prioridad alta de proceso y afinidad de núcleos configurable.

## 📥 Instalación

1. Descarga `RhythmBot-v0.1.0-alpha.zip`.
2. Descomprime en una carpeta.
3. Ejecuta `RhythmBot.exe calibrar` para configurar la región de captura.
4. Ajusta `Config.h` (o recompila) con los valores obtenidos.
5. Ejecuta `RhythmBot.exe` como **administrador**.

**Requisitos**:
- Windows 10/11 (64 bits)
- Ejecución como administrador (por `SendInput`)

## 🐛 Problemas conocidos

- **Algunos juegos con anti-cheat bloquean `SendInput`**. Se está investigando integración con driver Interception.
- **Notas largas (holds)** no soportadas todavía.
- **La UI es por consola** y archivo `Config.h`; UI gráfica en el roadmap.
- **La latencia depende del juego**: algunos necesitan ajuste manual del timing.

## ⚠️ Aviso legal

Este software puede violar los Términos de Servicio de juegos online. **Úsalo solo en juegos offline o entornos controlados.** El autor no se responsabiliza de sanciones o bloqueos de cuenta.

## 🛣️ Roadmap próximo

- [ ] Medidor automático de latencia
- [ ] Interfaz gráfica con ImGui
- [ ] Perfiles por juego (JSON)
- [ ] Soporte para holds
- [ ] Captura por DXGI

## 📜 Licencia

MIT License. Ver `LICENSE`.

## 🙏 Agradecimientos

- Comunidad de OpenCV por la librería de visión.
- Comunidad de vcpkg por el gestor de paquetes.
- Jugadores de rhythm games que dieron feedback.

---

**Binario**: `RhythmBot-v0.1.0-alpha.zip` (incluye DLLs de OpenCV)
**Código fuente**: descarga el ZIP del tag `v0.1.0-alpha`
