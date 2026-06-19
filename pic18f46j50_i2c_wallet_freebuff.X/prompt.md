
Actúa como un ingeniero electrónico experto en diseños de CNC y en el uso de KiCad 10. Tengo un proyecto existente en la carpeta "project_cnc_kicad/". Necesito que MODIFIQUES y COMPLETES el diseño según las siguientes especificaciones técnicas. El objetivo es crear un controlador CNC funcional con un microcontrolador PIC32.

### 1. Hardware Principal y Conexiones Obligatorias
- **Microcontrolador:** PIC32MX795F512 (100 pines, SMD).
    - **Acción:** Interconecta físicamente el uC con los 5 drivers Pololu.
- **Drivers Pololu:** 5 unidades (para ejes X, Y, Z, A, B).
    - **Conexión requerida por cada Pololu:**
        - **Paso (STEP):** Conectar a un pin del PIC32.
        - **Dirección (DIR):** Conectar a un pin del PIC32.
        - **Habilitar (ENABLE):** Conectar a un pin del PIC32.
        - **MS1, MS2, MS3 (Microstepping):** Conectar a pines del PIC32 (o a VCC/GND fijo).
        - **Alimentación del motor (VMOT):** Conectar a un terminal de tornillo aparte (NO a 3.3V).
        - **GND motor:** Conectar a un terminal de tornillo aparte.
- **Dimensiones de la PCB:** Estrictamente **96mm x 96mm**.
- **Número de capas:** 2 capas (doble cara).

### 2. Alimentación (Power Supply)
- **Entrada de Alimentación Principal:** Conector de tornillo (2 pines) para 12V-24V DC.
- **Regulador para Lógica (3.3V):** LM1117DT-3.3 (TO-252).
    - **Conexión:** Input desde el conector de tornillo, Output a la red de 3.3V.
- **Condensadores de Filtrado (Todos SMD 0805):**
    - Añadir condensadores de **10µF** y **0.1µF** en la entrada del regulador.
    - Añadir condensadores de **10µF** y **0.1µF** en la salida del regulador.
    - Añadir un **0.1µF** lo más cerca posible de cada pin de alimentación (VDD/VSS) del PIC32.
- **LED Indicador de Estado:** LED en la salida de 3.3V con su respectiva resistencia limitadora SMD 0805 (ej. 330Ω, 470Ω o 1K).

### 3. Comunicación USB
- **Conector:** USB Mini-B (SMD, tipo receptáculo).
- **Conexión:** Pines D+ y D- del conector conectados DIRECTAMENTE a los pines USB D+ y D- del PIC32.
- **Protección USB (Opcional pero recomendada):** Añadir espacio en el PCB para resistencias de protección de 22Ω-33Ω en serie en las líneas D+/D- (SMD 0805).

### 4. Componentes Pasivos y Formato (OBLIGATORIO)
- **Resistencias SMD 0805:** Usa este formato para todas las resistencias.
    - **CRÍTICO:** En el esquemático, añade un **comentario visible** a cada resistencia que diga su valor (ej. "10k", "330R", "1k").
- **Capacitores SMD 0805:** Usa este formato para todos los capacitores (excepto los electrolíticos de gran valor).
    - **CRÍTICO:** En el esquemático, añade un **comentario visible** a cada capacitor que diga su valor (ej. "100nF", "10µF").

### 5. Acciones Específicas a Realizar en los Archivos
1.  **Entrar a la carpeta `project_cnc_kicad/` y ANALIZAR el proyecto actual.**
2.  **MODIFICAR el esquemático:** Añade o corrige todas las conexiones mencionadas en los puntos 1, 2, 3 y 4.
3.  **CORREGIR** cualquier error o conexión faltante que detectes en el archivo original.
4.  **COMPLETAR** el diseño en Pcbnew para que cumpla con la medida de 96x96mm.
5.  **SUGERIR IDEAS:** Finalmente, en un comentario dentro del esquemático o en un archivo `notas_mejora.txt`, sugiere 3 ideas para mejorar el diseño (por ejemplo, optoaisladores para las entradas de límites, un conector para un display LCD, etc.).

**Formato de salida esperado:** Los archivos modificados y completados del proyecto KiCad 10 (.kicad_sch, .kicad_pcb, etc.) listos para ser guardados en la carpeta `project_cnc_kicad/`.

**¡Comienza!**
ruta : /home/optimus/Documentos/freebuff/project_cnc_kicad

project_cnc_kicad/
├── buttons.kicad_sch
├── cnc_pic32-backups
├── cnc_pic32.kicad_pcb
├── cnc_pic32.kicad_prl
├── cnc_pic32.kicad_pro
├── cnc_pic32.kicad_sch
├── cnc_pic32.net
├── fp-info-cache
├── ft232.kicad_sch
├── idc.kicad_sch
├── mcu.kicad_sch
├── osc_ft232.kicad_sch
├── oscilador.kicad_sch
├── oscilator.kicad_sch
├── pololu.kicad_sch
├── power_ft232.kicad_sch
├── power.kicad_sch
├── project.kicad_prl
├── ~project.kicad_pro.lck
├── tp_ft232.kicad_sch
├── usb_conector_Ft232.kicad_sch
├── Usb_pic32.kicad_sch
└── zigbee.kicad_sch


solo corregir el esquematico con sus intercionexiones , hacer todas las interconecciones jerargicas.
