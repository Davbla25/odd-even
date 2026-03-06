#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 

#define ARRAY_SIZE 50000 
#define TRACE_SIZE 50    // Tamaño más pequeño para que la animación en Python se vea bien

// ---------------------------------------------------------
// 1. VERSIÓN C ESTÁNDAR
// ---------------------------------------------------------
void odd_even_sort_c(int* arr, int n) {
    bool is_sorted = false;

    while (!is_sorted) {
        is_sorted = true; // Asumimos que está ordenado

        // Fase Par
        for (int j = 0; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                is_sorted = false; // Hubo un cambio, no estaba ordenado
            }
        }

        // Fase Impar
        for (int j = 1; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                is_sorted = false; // Hubo un cambio, no estaba ordenado
            }
        }
    }
}

// ---------------------------------------------------------
// 2. VERSIÓN ENSAMBLADOR X86 
// ---------------------------------------------------------
void odd_even_sort_asm(int* arr, int n) {
    if (n <= 1) return;

#if defined(_MSC_VER) && !defined(_WIN64)
    __asm {
        mov edi, arr; EDI = puntero base del array
        mov ecx, n
        dec ecx; ECX = n - 1 (limite para no salirnos del array)

        bucle_principal:
        mov ebx, 1; EBX sera nuestra bandera 'is_sorted' (1 = True, 0 = False)

            ; --- FASE PAR-- -
            mov esi, 0; j = 0
            bucle_par:
        cmp esi, ecx
            jge fin_fase_par; Si j >= n - 1, terminamos la fase par

            mov eax, [edi + esi * 4]; arr[j]
            mov edx, [edi + esi * 4 + 4]; arr[j + 1]
            cmp eax, edx
            jle no_swap_par; Si arr[j] <= arr[j + 1], no hacer nada

            ; Intercambiar
            mov[edi + esi * 4], edx
            mov[edi + esi * 4 + 4], eax
            mov ebx, 0; is_sorted = false (Hubo intercambio)

            no_swap_par:
        add esi, 2; j += 2
            jmp bucle_par

            fin_fase_par :

        ; --- FASE IMPAR-- -
            mov esi, 1; j = 1
            bucle_impar:
        cmp esi, ecx
            jge fin_fase_impar; Si j >= n - 1, terminamos la fase impar

            mov eax, [edi + esi * 4]; arr[j]
            mov edx, [edi + esi * 4 + 4]; arr[j + 1]
            cmp eax, edx
            jle no_swap_impar; Si arr[j] <= arr[j + 1], no hacer nada

            ; Intercambiar
            mov[edi + esi * 4], edx
            mov[edi + esi * 4 + 4], eax
            mov ebx, 0; is_sorted = false (Hubo intercambio)

            no_swap_impar:
        add esi, 2; j += 2
            jmp bucle_impar

            fin_fase_impar :

        ; --- COMPROBAR BANDERA-- -
            cmp ebx, 1
            jne bucle_principal; Si ebx no es 1 (es decir, hubo algun swap), repetimos todo

            fin :
    }
#else
    odd_even_sort_c(arr, n);
#endif
}

// ---------------------------------------------------------
// 3. GENERADOR DE TRAZADO VISUAL (Para Python)
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

    // Replicamos el algoritmo  para el archivo
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
    printf("[+] Archivo '%s' generado CON EXITO para la animacion en Python.\n", filename);
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
    // 1. Generar primero el archivo para la animacion en Python (Array pequeño)
    srand(42);
    generate_visual_trace("trace.txt", TRACE_SIZE);

    // 2. Preparar el Benchmark (Array inmenso)
    int* arr_c = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_asm = (int*)malloc(ARRAY_SIZE * sizeof(int));

    if (!arr_c || !arr_asm) {
        printf("Error asignando memoria.\n");
        return -1;
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val = rand() % 100000;
        arr_c[i] = val;
        arr_asm[i] = val;
    }

    printf("\n========================================\n");
    printf(" BENCHMARK ODD-EVEN: C vs ENSAMBLADOR\n");
    printf(" Elementos a ordenar: %d\n", ARRAY_SIZE);
    printf("========================================\n");

    clock_t start, end;
    double time_c, time_asm;

    // --- Medir C ---
    start = clock();
    odd_even_sort_c(arr_c, ARRAY_SIZE);
    end = clock();
    time_c = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[*] Tiempo C Estandar : %.4f segundos\n", time_c);

    // --- Medir Ensamblador ---
    start = clock();
    odd_even_sort_asm(arr_asm, ARRAY_SIZE);
    end = clock();
    time_asm = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[*] Tiempo x86 ASM    : %.4f segundos\n", time_asm);

    // --- Comprobación ---
    bool correct_c = is_sorted(arr_c, ARRAY_SIZE);
    bool correct_asm = is_sorted(arr_asm, ARRAY_SIZE);
    bool match = true;

    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (arr_c[i] != arr_asm[i]) {
            match = false;
            break;
        }
    }

    printf("----------------------------------------\n");
    if (correct_c && correct_asm && match) {
        printf("VERIFICADO: Ambos arrays ordenados con exito y coinciden.\n");
        if (time_asm > 0) {
            printf(">>> RENDIMIENTO: El codigo ASM es %.2fx en comparacion <<<\n", time_c / time_asm);
        }
    }
    else {
        printf("ERROR: Algo ha fallado en la ordenacion.\n");
    }
    printf("========================================\n");

    free(arr_c);
    free(arr_asm);

    printf("\nPresiona ENTER para salir...");
    getchar();
    return 0;
}