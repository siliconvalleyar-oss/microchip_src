/**
 * @file    custom_image_example.h
 * @brief   Ejemplo de cómo incluir una imagen personalizada
 * 
 * Para usar tu propia imagen:
 * 1. Convierte tu imagen usando: python3 image_converter.py tu_imagen.png
 * 2. Copia el archivo .h generado a tu proyecto
 * 3. Incluye este archivo en main.cpp
 * 4. Usa epd_loadImage(imageBW, convertedImage, sizeof(convertedImage));
 */

#ifndef CUSTOM_IMAGE_EXAMPLE_H
#define CUSTOM_IMAGE_EXAMPLE_H

#include <stdint.h>

// Ejemplo: imagen de patrón de prueba
static const uint8_t customPattern[5624] = {
    // Aquí irían los datos de tu imagen convertida
    // Generados automáticamente por image_converter.py
};

#endif /* CUSTOM_IMAGE_EXAMPLE_H */
