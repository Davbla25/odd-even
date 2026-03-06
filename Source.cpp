#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 
#include <intrin.h>      
#include <smmintrin.h>   // Librería para SSE4.1

#define ARRAY_SIZE 50000 
#define TRACE_SIZE 50    // Para el trazado visual de Python

// ---------------------------------------------------------
// 1. VERSIÓN C ESTÁNDAR (Optimizada con Early-Exit)
// ---------------------------------------------------------
void odd_even_sort_c(int* arr, int n) {
    bool is_sorted = false;
    while (!is_sorted) {
        is_sorted = true;
        for (int j = 0; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }
        for (int j = 1; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }
    }
}

// ---------------------------------------------------------
// 2. VERSIÓN ENSAMBLADOR X86 (Optimizada con Early-Exit)
// ---------------------------------------------------------
void odd_even_sort_asm(int* arr, int n) {
    if (n <= 1) return;

#if defined(_MSC_VER) && !defined(_WIN64)
    __asm {
        mov edi, arr; EDI = puntero base
        mov ecx, n
        dec ecx; ECX = n - 1

        bucle_principal:
        mov ebx, 1; ebx = is_sorted(1 = true)

            ; --- Fase Par-- -
            mov esi, 0
            bucle_par:
        cmp esi, ecx
            jge fin_fase_par

            mov eax, [edi + esi * 4]
            mov edx, [edi + esi * 4 + 4]
            cmp eax, edx
            jle no_swap_par

            mov[edi + esi * 4], edx
            mov[edi + esi * 4 + 4], eax
            mov ebx, 0; Hubo cambio->is_sorted = false

            no_swap_par:
        add esi, 2
            jmp bucle_par

            fin_fase_par :

        ; --- Fase Impar-- -
            mov esi, 1
            bucle_impar:
        cmp esi, ecx
            jge fin_fase_impar

            mov eax, [edi + esi * 4]
            mov edx, [edi + esi * 4 + 4]
            cmp eax, edx
            jle no_swap_impar

            mov[edi + esi * 4], edx
            mov[edi + esi * 4 + 4], eax
            mov ebx, 0; Hubo cambio->is_sorted = false

            no_swap_impar:
        add esi, 2
            jmp bucle_impar

            fin_fase_impar :
        cmp ebx, 1
            jne bucle_principal; Si ebx no es 1, repetir

            fin :
    }
#else
    odd_even_sort_c(arr, n);
#endif
}

// ---------------------------------------------------------
// 3. VERSIÓN SIMD SSE (Optimizada con Early-Exit)
// ---------------------------------------------------------
void odd_even_sort_sse(int* arr, int n) {
    bool is_sorted = false;
    while (!is_sorted) {
        is_sorted = true;

        // FASE PAR
        int j = 0;
        for (; j <= n - 4; j += 4) {
            __m128i v = _mm_loadu_si128((__m128i*) & arr[j]);
            __m128i v_swapped = _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1));
            __m128i v_min = _mm_min_epi32(v, v_swapped);
            __m128i v_max = _mm_max_epi32(v, v_swapped);
            __m128i v_res = _mm_blend_epi16(v_min, v_max, 0xCC);

            // Truco mágico: Comparar si el vector original y el ordenado son iguales
            __m128i cmp = _mm_cmpeq_epi32(v, v_res);
            if (_mm_movemask_epi8(cmp) != 0xFFFF) {
                is_sorted = false; // Si no son iguales, hubo un intercambio
            }

            _mm_storeu_si128((__m128i*) & arr[j], v_res);
        }
        for (; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }

        // FASE IMPAR
        j = 1;
        for (; j <= n - 4; j += 4) {
            __m128i v = _mm_loadu_si128((__m128i*) & arr[j]);
            __m128i v_swapped = _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1));
            __m128i v_min = _mm_min_epi32(v, v_swapped);
            __m128i v_max = _mm_max_epi32(v, v_swapped);
            __m128i v_res = _mm_blend_epi16(v_min, v_max, 0xCC);

            __m128i cmp = _mm_cmpeq_epi32(v, v_res);
            if (_mm_movemask_epi8(cmp) != 0xFFFF) {
                is_sorted = false;
            }

            _mm_storeu_si128((__m128i*) & arr[j], v_res);
        }
        for (; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }
    }
}

// ---------------------------------------------------------
// 4. GENERADOR DE TRAZADO (Para Pygame)
// ---------------------------------------------------------
void generate_visual_trace(const char* filename, int n) {
    FILE* f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "INIT %d\n", n);
    int* arr = (int*)malloc(n * sizeof(int));
    fprintf(f, "VALS");
    for (int i = 0; i < n; i++) {
        arr[i] = (rand() % 100) + 1;
        fprintf(f, " %d", arr[i]);
    }
    fprintf(f, "\n");

    bool is_sorted = false;
    while (!is_sorted) {
        is_sorted = true;
        for (int j = 0; j < n - 1; j += 2) {
            fprintf(f, "CMP %d %d\n", j, j + 1);
            if (arr[j] > arr[j + 1]) {
                fprintf(f, "SWP %d %d\n", j, j + 1);
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }
        for (int j = 1; j < n - 1; j += 2) {
            fprintf(f, "CMP %d %d\n", j, j + 1);
            if (arr[j] > arr[j + 1]) {
                fprintf(f, "SWP %d %d\n", j, j + 1);
                int temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
                is_sorted = false;
            }
        }
    }
    fprintf(f, "END\n");
    fclose(f);
    free(arr);
    printf("[+] Archivo 'trace.txt' generado para Pygame.\n");
}

// ---------------------------------------------------------
// UTILS: Verificación real de ordenación
// ---------------------------------------------------------
bool is_sorted(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) return false;
    }
    return true;
}

// ---------------------------------------------------------
// MAIN Y MEDICIÓN DE TIEMPOS
// ---------------------------------------------------------
int main() {
    srand(42);

    // 1. Generar traza visual
    generate_visual_trace("trace.txt", TRACE_SIZE);

    // 2. Preparar arrays para el Benchmark
    int* arr_c = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_asm = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_sse = (int*)malloc(ARRAY_SIZE * sizeof(int));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val = rand() % 100000;
        arr_c[i] = val;
        arr_asm[i] = val;
        arr_sse[i] = val;
    }

    printf("\n========================================\n");
    printf(" BENCHMARK ODD-EVEN: C vs ASM vs SSE\n");
    printf("========================================\n");

    clock_t start, end;
    double time_c, time_asm, time_sse;

    // --- Medir C ---
    start = clock();
    odd_even_sort_c(arr_c, ARRAY_SIZE);
    end = clock();
    time_c = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[*] Tiempo C Estandar   : %.4f segundos\n", time_c);

    // --- Medir Ensamblador ---
    start = clock();
    odd_even_sort_asm(arr_asm, ARRAY_SIZE);
    end = clock();
    time_asm = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[*] Tiempo Ensamblador  : %.4f segundos\n", time_asm);

    // --- Medir SSE ---
    start = clock();
    odd_even_sort_sse(arr_sse, ARRAY_SIZE);
    end = clock();
    time_sse = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[*] Tiempo SIMD (SSE)   : %.4f segundos\n", time_sse);

    // --- Comprobación ---
    bool correct_c = is_sorted(arr_c, ARRAY_SIZE);
    bool correct_asm = is_sorted(arr_asm, ARRAY_SIZE);
    bool correct_sse = is_sorted(arr_sse, ARRAY_SIZE);

    printf("----------------------------------------\n");
    if (correct_c && correct_asm && correct_sse) {
        printf("VERIFICADO: Los tres arrays ordenados con exito.\n");
        printf("\n>>> RESULTADOS DE RENDIMIENTO <<<\n");
        if (time_asm > 0) {
            printf("- ASM es %.2fx veces mas rapido que C\n", time_c / time_asm);
        }
        if (time_sse > 0) {
            printf("- SSE es %.2fx veces mas rapido que C\n", time_c / time_sse);
        }
    }
    else {
        printf("ERROR: Algo ha fallado en la ordenacion.\n");
    }
    printf("========================================\n");

    free(arr_c);
    free(arr_asm);
    free(arr_sse);

    printf("\nPresiona ENTER para salir...");
    getchar();
    return 0;
}