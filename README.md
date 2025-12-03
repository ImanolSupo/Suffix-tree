# Suffix Trees en C++

Repositorio con implementaciones de árbol de sufijos en C++:

- Ukkonen (O(n))
- Versión naive (O(n²))
- Dataset de prueba: `Bible.txt` (≈ 4.3M caracteres)

## Uso rápido

- Compilar y ejecutar los ejemplos de `main` para construir el árbol y hacer búsquedas de patrones.
- Ajustar el parámetro `limit` al cargar `Bible.txt` para controlar cuántos caracteres se usan.

## Advertencia de complejidad

- La versión naive solo es adecuada para textos pequeños.
- La versión Ukkonen es lineal, pero el árbol de sufijos completo sobre `Bible.txt` puede consumir mucha memoria y tardar en construirse.
