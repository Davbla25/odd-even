#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 
#include <intrin.h>      
#include <smmintrin.h>   // Librería para SSE4.1

#define ARRAY_SIZE 50000 
#define TRACE_SIZE 50    
#define NUM_RUNS 15      // Número de ejecuciones para calcular la media

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
// MAIN Y MEDICIÓN DE TIEMPOS ESTADÍSTICA
// ---------------------------------------------------------
int main() {
    srand(42);

    // 1. Generar traza visual para Python (se hace 1 sola vez)
    generate_visual_trace("trace.txt", TRACE_SIZE);

    // 2. Preparar arrays para el Benchmark
    int* arr_c = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_asm = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_sse = (int*)malloc(ARRAY_SIZE * sizeof(int));

    if (!arr_c || !arr_asm || !arr_sse) {
        printf("Error asignando memoria.\n");
        return -1;
    }

    printf("\n======================================================\n");
    printf("   BENCHMARK ODD-EVEN: C vs ASM vs SSE (SIMD)\n");
    printf("   Elementos a ordenar : %d\n", ARRAY_SIZE);
    printf("   Numero de pruebas   : %d\n", NUM_RUNS);
    printf("======================================================\n\n");

    double total_time_c = 0.0, total_time_asm = 0.0, total_time_sse = 0.0;
    bool all_correct = true;

    // --- BUCLE DE PRUEBAS ---
    for (int run = 1; run <= NUM_RUNS; run++) {

        // Rellenar con nuevos datos aleatorios para esta pasada
        // Así nos aseguramos de que no están ordenando un array ya ordenado
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int val = rand() % 100000;
            arr_c[i] = val;
            arr_asm[i] = val;
            arr_sse[i] = val;
        }

        clock_t start, end;
        double t_c, t_asm, t_sse;

        // Medir C
        start = clock();
        odd_even_sort_c(arr_c, ARRAY_SIZE);
        end = clock();
        t_c = (double)(end - start) / CLOCKS_PER_SEC;
        total_time_c += t_c;

        // Medir ASM
        start = clock();
        odd_even_sort_asm(arr_asm, ARRAY_SIZE);
        end = clock();
        t_asm = (double)(end - start) / CLOCKS_PER_SEC;
        total_time_asm += t_asm;

        // Medir SSE
        start = clock();
        odd_even_sort_sse(arr_sse, ARRAY_SIZE);
        end = clock();
        t_sse = (double)(end - start) / CLOCKS_PER_SEC;
        total_time_sse += t_sse;

        // Mostrar progreso por pantalla
        printf("[Prueba %2d] C: %.4fs | ASM: %.4fs | SSE: %.4fs\n", run, t_c, t_asm, t_sse);

        // Verificación de seguridad en cada pasada
        if (!is_sorted(arr_c, ARRAY_SIZE) || !is_sorted(arr_asm, ARRAY_SIZE) || !is_sorted(arr_sse, ARRAY_SIZE)) {
            all_correct = false;
        }
    }

    // --- CÁLCULO DE MEDIAS ---
    double avg_c = total_time_c / NUM_RUNS;
    double avg_asm = total_time_asm / NUM_RUNS;
    double avg_sse = total_time_sse / NUM_RUNS;

    printf("\n------------------------------------------------------\n");
    if (all_correct) {
        printf(" [VERIFICADO] Todos los arrays ordenados correctamente.\n");
        printf("------------------------------------------------------\n");
        printf(">>> TIEMPO MEDIO DE EJECUCION <<<\n");
        printf("[*] C Estandar  : %.4f segundos\n", avg_c);
        printf(" [*] Ensamblador : %.4f segundos\n", avg_asm);
        printf(" [*] SIMD (SSE)  : %.4f segundos\n", avg_sse);

        printf("\n>>> COMPARATIVA DE RENDIMIENTO (SPEEDUP) <<<\n");
        if (avg_asm > 0) {
            printf(" - Ganancia ASM : %.2fx veces mas rapido que C\n", avg_c / avg_asm);
        }
        if (avg_sse > 0) {
            printf(" - Ganancia SSE : %.2fx veces mas rapido que C\n", avg_c / avg_sse);
        }
    }
    else {
        printf(" ERROR CRITICO: Al menos una ordenacion ha fallado.\n");
    }
    printf("======================================================\n");

    free(arr_c);
    free(arr_asm);
    free(arr_sse);

    printf("\nPresiona ENTER para salir...");
    getchar();
    return 0;
}