#!/usr/bin/env python3
"""
Conversor de imágenes a formato binario para E-Paper display 2.66"

Uso:
    python3 image_converter.py imagen.png imagen.h

La imagen debe ser monocromática (blanco y negro) o se convertirá a 1bpp.
Tamaño recomendado: 296x152 píxeles
"""

import sys
from PIL import Image
import os

def convert_image_to_epaper(input_path, output_path):
    """Convierte una imagen a formato binario para E-Paper"""
    
    # Cargar imagen
    img = Image.open(input_path)
    
    # Convertir a escala de grises
    if img.mode != 'L':
        img = img.convert('L')
    
    # Redimensionar a 296x152 (manteniendo aspecto)
    target_size = (296, 152)
    img = img.resize(target_size, Image.Resampling.LANCZOS)
    
    # Convertir a 1bpp (umbral 128)
    img = img.point(lambda p: 255 if p > 128 else 0, '1')
    
    # Obtener datos en modo '1' (1-bit píxeles)
    pixels = img.load()
    
    # Calcular tamaño del buffer
    width, height = img.size
    bytes_per_row = width // 8
    buffer_size = bytes_per_row * height
    
    # Generar array C
    c_array = []
    c_array.append(f"// Imagen convertida: {os.path.basename(input_path)}")
    c_array.append(f"// Tamaño: {width}x{height}, {buffer_size} bytes")
    c_array.append("")
    c_array.append("#ifndef CONVERTED_IMAGE_H")
    c_array.append("#define CONVERTED_IMAGE_H")
    c_array.append("")
    c_array.append("#include <stdint.h>")
    c_array.append("")
    c_array.append(f"static const uint8_t convertedImage[{buffer_size}] = {{")
    
    # Procesar cada fila
    for y in range(height):
        row_bytes = []
        for x_byte in range(bytes_per_row):
            byte_val = 0
            for bit in range(8):
                x = x_byte * 8 + bit
                if x < width:
                    # Negro = 1, Blanco = 0
                    if pixels[x, y] == 0:  # Negro en modo '1'
                        byte_val |= (0x80 >> bit)
            row_bytes.append(f"0x{byte_val:02X}")
        
        # Formatear línea con 16 bytes por línea para mejor legibilidad
        line = "    " + ", ".join(row_bytes)
        if y < height - 1:
            line += ","
        c_array.append(line)
    
    c_array.append("};")
    c_array.append("")
    c_array.append("#endif /* CONVERTED_IMAGE_H */")
    
    # Escribir archivo
    with open(output_path, 'w') as f:
        f.write("\n".join(c_array))
    
    print(f"✓ Imagen convertida: {output_path}")
    print(f"  - Dimensiones: {width}x{height}")
    print(f"  - Tamaño buffer: {buffer_size} bytes")

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 image_converter.py <imagen_entrada> [imagen_salida.h]")
        print("Ejemplo: python3 image_converter.py logo.png my_image.h")
        sys.exit(1)
    
    input_path = sys.argv[1]
    
    if len(sys.argv) >= 3:
        output_path = sys.argv[2]
    else:
        base_name = os.path.splitext(input_path)[0]
        output_path = base_name + ".h"
    
    if not os.path.exists(input_path):
        print(f"Error: No se encuentra el archivo {input_path}")
        sys.exit(1)
    
    try:
        convert_image_to_epaper(input_path, output_path)
    except Exception as e:
        print(f"Error al convertir la imagen: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
