# pic18f452_MAF

Sensor MAF (Mass Air Flow) para PIC18F452

## Microcontrolador

- **PIC18F452**

## Estructura del proyecto

```
pic18f452_MAF.X/
├── src/           # Código fuente
├── inc/           # Archivos de cabecera
├── examples/      # Ejemplos de uso
├── nbproject/     # Configuración de MPLAB X
├── Makefile       # Reglas de compilación
└── README.md      # Este archivo
```

## Dependencias

- MPLAB X IDE
- XC32 Compiler (para PIC32MX) / XC8 Compiler (para PIC18F)
- (Otras dependencias específicas si aplican)

## Pines utilizados

```
// Configuración de pines - ver archivos de configuración en inc/
```

## Compilación

Abrir el proyecto en MPLAB X IDE o compilar desde línea de comandos:

```bash
cd pic18f452_MAF.X
make -f Makefile
```

## Programación

Conectar el programador (PICkit, ICD, etc.) y desde MPLAB X seleccionar "Make and Program Device".

## Licencia

MIT License - ver LICENSE para detalles.
