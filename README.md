# Game Boy Emulator (`gameboy-emu`)

Un emulador de Nintendo Game Boy en desarrollo, construido desde cero con un enfoque en comprender la arquitectura de hardware de 8 bits, la gestión de memoria y el ciclo de instrucciones a nivel de bajo nivel.

## Estado del Proyecto: En Desarrollo 🚧

Actualmente, el emulador se encuentra en fase de implementación de sus componentes principales. 
* **CPU (Sharp LR35902):** En desarrollo / integración de instrucciones.
* **MMU (Memory Management Unit):** Estructurada para el manejo del mapa de memoria de 16 bits.
* **PPU (Picture Processing Unit):** ⚠️ **Pendiente de implementación.** (Próximo objetivo principal: renderizado de gráficos, modos de pantalla y tiles).

## Arquitectura Prevista

El emulador está diseñado bajo un modelo modular para simular los subsistemas originales de la consola:

1. **CPU (Procesador):** Ejecución del ciclo *Fetch-Decode-Execute*, manejo de registros de 8 y 16 bits, y banderas (*flags*).
2. **Memory Map (MMU/Bus):** Gestión de los rangos de memoria (ROM, RAM de trabajo, VRAM, OAM y registros de I/O).
3. **PPU (Gráficos):** Sincronización de los modos de la pantalla LCD (H-Blank, V-Blank, OAM Scan, Pixel Transfer) y renderizado de *tiles* y sprites. *(En construcción)*
4. **Timers & Interrupts:** Manejo de interrupciones de hardware (V-Blank, Timer, Serial, Joypad).

## Requisitos y Dependencias *(Sujeto a cambios)*
* Compilador compatible con C/C++ (GCC / Clang / MSVC) o el lenguaje de tu elección.
* Make / CMake.

## Próximos Pasos
- [ ] Finalizar el set completo de instrucciones de la CPU.
- [ ] Implementar la PPU y el pipeline de renderizado de píxeles.
- [ ] Añadir soporte para lectura de ROMs de prueba (Blargg's test ROMs).
- [ ] Integrar un backend gráfico (como SDL2) para la ventana de visualización.

## Licencia
Este proyecto se distribuye bajo la [Licencia MIT](LICENSE).
