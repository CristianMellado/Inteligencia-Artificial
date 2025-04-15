#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <cstdlib>
#include <ctime>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NODE_RADIUS 20.0f
#define MAX_COLORS 3

float COLORS[MAX_COLORS][3] = {
    {1.0f, 0.0f, 0.0f}, //rojo
    {0.0f, 1.0f, 0.0f}, //verde
    {0.0f, 0.0f, 1.0f}  //azul
};

float GRAY[3] = { 0.5f, 0.5f, 0.5f };

int heuristic = 1;
int backtrackCount = 0;

struct Nodo {
    int id;
    float x, y;
    int colorIndex; //-1 = sin color
    std::vector<int> vecinos;
};

struct Grafo {
    std::vector<Nodo> nodos;
    std::map<int, std::vector<int>> adyacencia;

    void agregarNodo(float x, float y) {
        Nodo nodo;
        nodo.id = nodos.size();
        nodo.x = x;
        nodo.y = y;
        nodo.colorIndex = -1; //gris al inicio
        nodos.push_back(nodo);
    }

    bool esColorValido(int idNodo, int color) {
        for (int vecino : nodos[idNodo].vecinos) {
            if (nodos[vecino].colorIndex == color) {
                return false;
            }
        }
        return true;
    }

    std::vector<int> coloresDisponibles(int idNodo) {
        std::vector<int> disponibles;
        for (int c = 0; c < MAX_COLORS; ++c) {
            if (esColorValido(idNodo, c)) {
                disponibles.push_back(c);
            }
        }
        return disponibles;
    }

    void conectarNodos(int id1, int id2) {
        ///
        int nodoA = id1;
        int nodoB = id2;

        if (heuristic == 1) { //mas restrictiva: nodo con menos colores disponibles
            if (coloresDisponibles(id1).size() < coloresDisponibles(id2).size()) {
                nodoA = id1;
                nodoB = id2;
            }
            else {
                nodoA = id2;
                nodoB = id1;
            }
        }
        else if (heuristic == 2) { //mas restringida: nodo con más vecinos
            if (nodos[id1].vecinos.size() > nodos[id2].vecinos.size()) {
                nodoA = id1;
                nodoB = id2;
            }
            else {
                nodoA = id2;
                nodoB = id1;
            }
        }
        ////
        //asigna color valido al nodoA si aun no tiene
        if (nodos[nodoA].colorIndex == -1) {
            std::vector<int> opcionesA = coloresDisponibles(nodoA);
            if (!opcionesA.empty()) {
                nodos[nodoA].colorIndex = colorMenosUsado(opcionesA);///
            }
            else {
                backtrackCount++; //contador de fallos cuando no hay color valido
                std::cout << "\n no hay colores disponibles para el nodo " << nodoA << ".\n";
            }
        }

        //asigna color valido al nodoB si aun no tiene
        if (nodos[nodoB].colorIndex == -1) {
            std::vector<int> opcionesB = coloresDisponibles(nodoB);
            if (!opcionesB.empty()) {
                nodos[nodoB].colorIndex = colorMenosUsado(opcionesB);///
            }
            else {
                backtrackCount++;
                std::cout << "\nno hay colores disponibles para el nodo " << nodoB << ".\n";
            }
        }///

        nodos[nodoA].vecinos.push_back(nodoB);
        nodos[nodoB].vecinos.push_back(nodoA);
        adyacencia[nodoA].push_back(nodoB);
        adyacencia[nodoB].push_back(nodoA);

        // Si nodoA no tenía color, asignarle uno válido
        if (nodos[nodoA].colorIndex == -1) {
            std::vector<int> dispA = coloresDisponibles(nodoA);
            if (!dispA.empty()) {
                nodos[nodoA].colorIndex = dispA[0];
            }
        }
    }

    int nodoClickeado(float mx, float my) {
        for (auto& nodo : nodos) {
            float dx = nodo.x - mx;
            float dy = nodo.y - my;
            if (sqrt(dx * dx + dy * dy) <= NODE_RADIUS)
                return nodo.id;
        }
        return -1;
    }
    ///
    int colorMenosUsado(const std::vector<int>& opciones) {
        std::map<int, int> usoColor;

        //cuenta las veces q se usa un color
        for (auto& nodo : nodos) {
            if (nodo.colorIndex != -1)
                usoColor[nodo.colorIndex]++;
        }
        //encuentra el color menos usado entre las opciones
        int mejorColor = opciones[0];
        int menorUso = usoColor[mejorColor];

        for (int color : opciones) {
            int uso = usoColor[color];
            if (uso < menorUso) {
                mejorColor = color;
                menorUso = uso;
            }
        }
        return mejorColor;
    }

};

Grafo grafo;
int nodoSeleccionado = -1;

void dibujarCirculo(float cx, float cy, float r, float color[3]) {
    glColor3fv(color);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 100; i++) {
        float angle = 2.0f * 3.1415926f * i / 100;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        mx = (float)mx;
        my = WINDOW_HEIGHT - (float)my; //

        int id = grafo.nodoClickeado(mx, my);
        if (id == -1) {
            grafo.agregarNodo(mx, my);
        }
        else {
            if (nodoSeleccionado == -1) {
                nodoSeleccionado = id;
            }
            else {
                if (id != nodoSeleccionado) {
                    grafo.conectarNodos(nodoSeleccionado, id);
                }
                nodoSeleccionado = -1;
            }
        }
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& nodo : grafo.nodos) {
        if (nodo.colorIndex == -1) {
            dibujarCirculo(nodo.x, nodo.y, NODE_RADIUS, GRAY);
        }
        else {
            float* color = COLORS[nodo.colorIndex];
            dibujarCirculo(nodo.x, nodo.y, NODE_RADIUS, color);
        }
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    std::cout << "Seleccione heuristica:\n";
    std::cout << "1. Variable mas restrictiva\n";
    std::cout << "2. Variable mas restringida\n";
    std::cin >> heuristic;

    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Coloreo de Grafos", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Cantidad de veces que se hizo backtracking: " << backtrackCount << "\n";
    return 0;
}
