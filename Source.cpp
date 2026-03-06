#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 

#define ARRAY_SIZE 50000 

// ---------------------------------------------------------
// 1. VERSIÓN C ESTÁNDAR (Línea Base)
// ---------------------------------------------------------
void odd_even_sort_c(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < n - 1; j += 2) {
                if (arr[j] > arr[j + 1]) {
                    int temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
        else {
            for (int j = 1; j < n - 1; j += 2) {
                if (arr[j] > arr[j + 1]) {
                    int temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
    }
}

// ---------------------------------------------------------
// 2. VERSIÓN ENSAMBLADOR X86 
// ---------------------------------------------------------
void odd_even_sort_asm(int* arr, int n) {
    // Si el array tiene 1 o menos elementos, no hay que ordenar
    if (n <= 1) return;

    
#if defined(_MSC_VER) && !defined(_WIN64)
    __asm {
        mov edi, arr; EDI = puntero base del array
        mov ebx, 0; EBX = i = 0 (Contador del bucle externo)

        bucle_ext:
        mov eax, n
            cmp ebx, eax
            jge fin; Si i >= n, terminamos

            ; Comprobar si 'i' es par o impar(i % 2)
            mov eax, ebx
            and eax, 1; AND a nivel de bits.Si el bit 0 es 0, es par.
            jz fase_par; Salta si es cero(Z = 1)

            fase_impar:
            mov esi, 1; ESI = j = 1
            jmp bucle_int

            fase_par :
            mov esi, 0; ESI = j = 0

            bucle_int:
            mov eax, n
            dec eax; EAX = n - 1
            cmp esi, eax
            jge fin_bucle_int; Si j >= n - 1, terminamos la pasada

            ; Modo de direccionamiento : Base + Indice * Escala(Pág. 16 de la guía)
            ; Cada int ocupa 4 bytes, por eso se multiplica por 4.
            mov ecx, [edi + esi * 4]; Carga arr[j] en ECX
            mov edx, [edi + esi * 4 + 4]; Carga arr[j + 1] en EDX

            cmp ecx, edx
            jle no_swap; Si arr[j] <= arr[j + 1], no hace falta intercambiar

            ; Intercambio de variables(SWAP)
            mov[edi + esi * 4], edx
            mov[edi + esi * 4 + 4], ecx

            no_swap :
            add esi, 2; j += 2
            jmp bucle_int

            fin_bucle_int :
            inc ebx; i++
            jmp bucle_ext

            fin :
    }
#else
    // Si compilas en 64 bits por error, el ensamblador inline no funcionará.
    // Llamamos a la versión de C para que no reviente el programa.
    printf("[!] ADVERTENCIA: Compilado en 64-bits. Ejecutando version C en su lugar.\n");
    odd_even_sort_c(arr, n);
#endif
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
// 3. MAIN Y MEDICIÓN DE TIEMPOS
// ---------------------------------------------------------
int main() {
    // Memoria estándar (sin librerías raras)
    int* arr_c = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr_asm = (int*)malloc(ARRAY_SIZE * sizeof(int));

    if (!arr_c || !arr_asm) {
        printf("Error asignando memoria.\n");
        return -1;
    }

    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val = rand() % 100000;
        arr_c[i] = val;
        arr_asm[i] = val;
    }

    printf("========================================\n");
    printf(" BENCHMARK ODD-EVEN: C vs ENSAMBLADOR\n");
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
        printf("VERIFICADO: Ambos arrays ordenados con exito.\n");
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