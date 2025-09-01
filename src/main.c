/**
 * \file main.c
 * \brief Visualização de trigonometria esférica aplicada a sensores em aeronaves (Raylib).
 *
 * Este programa demonstra, de forma didática e interativa, como converter
 * coordenadas angulares de \b azimute (Az) e \b elevação (El) em vetores 3D
 * unitários sobre uma \b esfera unitária. A partir de dois vetores, o do
 * \b alvo (T) e o do \b eixo de rolagem (R) da aeronave, calculamos o
 * \b ângulo esférico J, isto é, o ângulo do \b grande círculo entre T e R.
 *
 * Conceitos principais (linguagem simples):
 * - A esfera unitária tem raio 1. Cada direção no espaço pode ser vista como
 *   um ponto na superfície dessa esfera.
 * - Azimute (Az): ângulo no plano horizontal, medido de +X ("Norte") para +Y
 *   ("Leste").
 * - Elevação (El): ângulo medido do plano XY (horizonte) para +Z ("para cima").
 * - Vetor unitário: vetor de comprimento 1 apontando na direção desejada.
 * - Ângulo J: menor ângulo entre os dois vetores \c vT e \c vR, medido no
 *   centro da esfera (ângulo central), equivalente ao arco de grande círculo
 *   conectando as duas direções na esfera.
 *
 * Duas formas de calcular J:
 * 1) Produto escalar (robusta numericamente):  \f$J = \arccos(\operatorname{clamp}(v_T\cdot v_R, -1, 1))\f$.
 * 2) Fórmula analítica (lei dos cossenos esférica):
 *    \f$\cos J = \sin(El_T)\sin(El_R) + \cos(El_T)\cos(El_R)\cos(Az_T - Az_R)\f$.
 * Ambas devem coincidir (até erros de arredondamento).
 *
 * Controles:
 * - Alvo (T): A/D = Az −/+  |  W/S = El +/−
 * - Eixo (R): J/L = Az −/+  |  I/K = El +/−
 * - Reset: R
 * - Mouse (botão direito): orbitar câmera  |  Scroll: FOV
 */
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float deg2rad(float d) { return d * (float)M_PI / 180.0f; }

// Forward declaration (usada antes da definição)
static Vector3 AzElToVec(float az, float el);

/**
 * \brief Desenha um arco de azimute no plano do horizonte (El=0) de az0 até az1.
 */
static void DrawAzimuthArc(float az0, float az1, Color color) {
    int steps = 64;
    float r = 1.001f; // levemente acima da esfera para evitar z-fighting
    float a0 = az0, a1 = az1;
    // manter direção (se az1 < az0, desenha no sentido negativo)
    for (int i = 0; i < steps; ++i) {
        float t0 = (float)i/steps;
        float t1 = (float)(i+1)/steps;
        float aA = a0 + (a1 - a0)*t0;
        float aB = a0 + (a1 - a0)*t1;
        Vector3 pA = (Vector3){ r*cosf(aA), r*sinf(aA), 0.0f };
        Vector3 pB = (Vector3){ r*cosf(aB), r*sinf(aB), 0.0f };
        DrawLine3D(pA, pB, color);
    }
}

/**
 * \brief Desenha um arco de elevação, para az fixo, de 0 até el.
 */
static void DrawElevationArc(float az, float el, Color color) {
    int steps = 32;
    float r = 1.001f;
    for (int i = 0; i < steps; ++i) {
        float t0 = (float)i/steps;
        float t1 = (float)(i+1)/steps;
        float eA = 0.0f + (el - 0.0f)*t0;
        float eB = 0.0f + (el - 0.0f)*t1;
        Vector3 pA = AzElToVec(az, eA);
        Vector3 pB = AzElToVec(az, eB);
        pA = Vector3Scale(pA, r);
        pB = Vector3Scale(pB, r);
        DrawLine3D(pA, pB, color);
    }
}

/**
 * \brief Slerp (interpolação esférica) entre dois vetores unitários.
 */
static Vector3 SlerpUnit(Vector3 a, Vector3 b, float t) {
    float dot = a.x*b.x + a.y*b.y + a.z*b.z;
    if (dot > 1.0f) dot = 1.0f; else if (dot < -1.0f) dot = -1.0f;
    float theta = acosf(dot);
    if (theta < 1e-5f) return a; // quase iguais
    float s = sinf(theta);
    float w0 = sinf((1.0f - t)*theta)/s;
    float w1 = sinf(t*theta)/s;
    Vector3 r0 = Vector3Scale(a, w0);
    Vector3 r1 = Vector3Scale(b, w1);
    return Vector3Add(r0, r1);
}

/**
 * \brief Desenha o arco de grande círculo entre dois vetores unitários (ângulo j).
 */
static void DrawGreatCircleArc(Vector3 a, Vector3 b, Color color) {
    int steps = 64;
    float r = 1.002f;
    Vector3 prev = Vector3Scale(a, r);
    for (int i = 1; i <= steps; ++i) {
        float t = (float)i/steps;
        Vector3 cur = SlerpUnit(a, b, t);
        cur = Vector3Scale(cur, r);
        DrawLine3D(prev, cur, color);
        prev = cur;
    }
}
static float rad2deg(float r) { return r * 180.0f / (float)M_PI; }

/**
 * \brief Converte azimute/elevação (rad) em um vetor 3D unitário.
 *
 * Sistema de eixos adotado:
 * - +X: "Norte" (referência de azimute 0°)
 * - +Y: "Leste"  (aumenta com o azimute)
 * - +Z: "Cima"   (aumenta com a elevação)
 *
 * Fórmulas usadas (derivadas de coordenadas esféricas):
 * - \f$x = \cos(El)\cos(Az)\f$
 * - \f$y = \cos(El)\sin(Az)\f$
 * - \f$z = \sin(El)\f$
 *
 * O vetor é normalizado por segurança numérica (deve ter norma 1).
 *
 * \param az Azimute em radianos.
 * \param el Elevação em radianos.
 * \return Vetor 3D unitário correspondente à direção (az, el).
 */
static Vector3 AzElToVec(float az, float el) {
    float ce = cosf(el);
    Vector3 v = { ce * cosf(az), ce * sinf(az), sinf(el) };
    float n = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (n > 0.0f) {
        v.x /= n; v.y /= n; v.z /= n;
    }
    return v;
}

/**
 * \brief Calcula o ângulo (rad) entre dois vetores unitários.
 *
 * Para vetores unitários \c a e \c b, o produto escalar \c a·b é igual a
 * \f$\cos(\theta)\f$, onde \f$\theta\f$ é o ângulo entre os vetores. Fazemos
 * um \c clamp do valor para o intervalo [-1, 1] para evitar erros numéricos
 * (por exemplo, arredondamentos que produziriam \c acos fora do domínio).
 *
 * \param a Vetor unitário A.
 * \param b Vetor unitário B.
 * \return Ângulo entre A e B, em radianos (intervalo [0, \f$\pi\f$]).
 */
static float AngleBetweenUnit(Vector3 a, Vector3 b) {
    float c = a.x*b.x + a.y*b.y + a.z*b.z;
    if (c > 1.0f) c = 1.0f; else if (c < -1.0f) c = -1.0f;
    return acosf(c);
}

/**
 * \brief Desenha uma seta 3D simples entre dois pontos.
 *
 * A haste é uma linha e a ponta é um pequeno cone. Este é apenas um recurso
 * visual para indicar direção no espaço; não altera nenhum cálculo.
 *
 * \param start Ponto inicial da seta.
 * \param end Ponto final (direção) da seta.
 * \param thickness Controle da espessura/raio da ponta.
 * \param color Cor da seta.
 */
static void DrawArrow3D(Vector3 start, Vector3 end, float thickness, Color color) {
    DrawLine3D(start, end, color);
    Vector3 dir = Vector3Subtract(end, start);
    float len = Vector3Length(dir);
    if (len < 1e-4f) return;
    Vector3 ndir = Vector3Scale(dir, 1.0f/len);
    float headLen = fminf(0.25f*len, 0.5f);
    float headRad = thickness*2.0f;
    Vector3 base = Vector3Add(end, Vector3Scale(ndir, -headLen));
    DrawCylinderEx(base, end, headRad*0.5f, 0.0f, 12, color);
}

/**
 * \brief Desenha uma esfera aramada (wireframe) para referência visual.
 *
 * A esfera unitária é muito útil para enxergarmos cada direção (vetor unitário)
 * como um ponto na sua superfície. Aqui desenhamos "paralelos" (linhas de
 * elevação) e "meridianos" (linhas de azimute).
 *
 * \param radius Raio da esfera (tipicamente 1.0).
 * \param segAzi Quantidade de segmentos em azimute (meridianos).
 * \param segEle Quantidade de segmentos em elevação (paralelos).
 * \param color Cor das linhas.
 */
static void DrawSphereWire(float radius, int segAzi, int segEle, Color color) {
    // Linhas de latitude (elevação)
    for (int i = 1; i < segEle; ++i) {
        float t = (float)i/segEle * (float)M_PI; // 0..pi
        float z = cosf(t);
        float r = sinf(t);
        Vector3 prev = { radius*r, 0, radius*z };
        for (int k = 1; k <= segAzi; ++k) {
            float a = (float)k/segAzi * 2.0f*(float)M_PI;
            Vector3 cur = (Vector3){ radius*r*cosf(a), radius*r*sinf(a), radius*z };
            DrawLine3D(prev, cur, Fade(color, 0.4f));
            prev = cur;
        }
    }
    // Linhas de longitude (azimute)
    for (int k = 0; k < segAzi; ++k) {
        float a = (float)k/segAzi * 2.0f*(float)M_PI;
        Vector3 prev = { radius*cosf(a), 0, 0 };
        for (int i = 1; i <= segEle; ++i) {
            float t = (float)i/segEle * (float)M_PI;
            Vector3 cur = (Vector3){ radius*sinf(t)*cosf(a), radius*sinf(t)*sinf(a), radius*cosf(t) };
            DrawLine3D(prev, cur, Fade(color, 0.4f));
            prev = cur;
        }
    }
}

/**
 * \brief Função principal. Configura a janela/câmera e executa o laço de renderização.
 *
 * Passos do laço (cada quadro):
 * 1. Lê o teclado e atualiza os ângulos do alvo (T) e do eixo (R).
 * 2. Converte (Az, El) em vetores unitários \c vT e \c vR com \ref AzElToVec.
 * 3. Calcula o ângulo esférico \c J pelo produto escalar (\ref AngleBetweenUnit).
 * 4. Calcula \c J também pela fórmula analítica (verificação de consistência).
 * 5. Desenha eixos, esfera, setas dos vetores e um HUD com os valores.
 *
 * Observações:
 * - A escolha de eixos (X=Norte, Y=Leste, Z=Cima) é uma convenção. Você pode
 *   adaptá-la ao seu sistema, desde que ajuste as fórmulas de conversão.
 * - As duas formas de calcular \c J devem coincidir. Pequenas diferenças
 *   ocorrem por arredondamentos de ponto flutuante (isso é esperado).
 */
int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Trigonometria Esférica — Az/El & Ângulo J");

    Camera3D cam = {0};
    cam.position = (Vector3){ 2.5f, 2.0f, 2.5f };
    cam.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
    cam.up       = (Vector3){ 0.0f, 0.0f, 1.0f }; // Z para cima
    cam.fovy     = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    // Parâmetros iniciais (em graus)
    float azT_deg = 40.0f, elT_deg = 25.0f; // alvo
    float azR_deg = 10.0f, elR_deg =  5.0f; // eixo de rolagem

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Controles
        float dt = GetFrameTime();
        float sp = 60.0f * dt; // deg/s
        // Alvo T
        if (IsKeyDown(KEY_A)) azT_deg -= sp; if (IsKeyDown(KEY_D)) azT_deg += sp;
        if (IsKeyDown(KEY_W)) elT_deg += sp; if (IsKeyDown(KEY_S)) elT_deg -= sp;
        // Eixo R
        if (IsKeyDown(KEY_J)) azR_deg -= sp; if (IsKeyDown(KEY_L)) azR_deg += sp;
        if (IsKeyDown(KEY_I)) elR_deg += sp; if (IsKeyDown(KEY_K)) elR_deg -= sp;
        // Reset
        if (IsKeyPressed(KEY_R)) { azT_deg=40; elT_deg=25; azR_deg=10; elR_deg=5; }

        // Limites razoáveis para elevação (-89..+89)
        if (elT_deg > 89) elT_deg = 89; if (elT_deg < -89) elT_deg = -89;
        if (elR_deg > 89) elR_deg = 89; if (elR_deg < -89) elR_deg = -89;

        // Câmera orbital simples
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 d = GetMouseDelta();
            static float yaw = 0, pitch = 0.6f;
            yaw   += d.x * 0.003f;
            pitch += d.y * 0.003f;
            if (pitch > 1.3f) pitch = 1.3f;
            if (pitch < -0.1f) pitch = -0.1f;
            float r = 3.5f;
            cam.position.x = r * cosf(pitch) * cosf(yaw);
            cam.position.y = r * cosf(pitch) * sinf(yaw);
            cam.position.z = r * sinf(pitch);
        }
        float mw = GetMouseWheelMove();
        if (fabsf(mw) > 0.01f) cam.fovy = Clamp(cam.fovy - mw*2.0f, 20.0f, 90.0f);

        // Cálculos
        float AZ_T = deg2rad(azT_deg), EL_T = deg2rad(elT_deg);
        float AZ_R = deg2rad(azR_deg), EL_R = deg2rad(elR_deg);
        Vector3 vT = AzElToVec(AZ_T, EL_T);
        Vector3 vR = AzElToVec(AZ_R, EL_R);
        float J = AngleBetweenUnit(vT, vR);
        float Jdeg = rad2deg(J);

        // Verificação forma analítica: cosJ = sin(EL_T)sin(EL_R)+cos(EL_T)cos(EL_R)cos(ΔAZ)
        float cosJ = sinf(EL_T)*sinf(EL_R) + cosf(EL_T)*cosf(EL_R)*cosf(AZ_T - AZ_R);
        if (cosJ > 1.0f) cosJ = 1.0f; else if (cosJ < -1.0f) cosJ = -1.0f;
        float Jdeg_trig = rad2deg(acosf(cosJ));

        BeginDrawing();
        ClearBackground((Color){20,24,28,255});

        BeginMode3D(cam);
        // Eixos N-E-Up (X=North, Y=East, Z=Up)
        float L = 1.2f;
        DrawLine3D((Vector3){0,0,0}, (Vector3){L,0,0}, WHITE);
        DrawLine3D((Vector3){0,0,0}, (Vector3){0,L,0}, WHITE);
        DrawLine3D((Vector3){0,0,0}, (Vector3){0,0,L}, WHITE);

        // Esfera unitária (grade em tom alaranjado)
        DrawSphereWire(1.0f, 32, 20, Fade(ORANGE, 0.35f));
        // Equador completo (destaque)
        DrawAzimuthArc(0.0f, 2.0f*(float)M_PI, Fade(ORANGE, 0.55f));

        // Vetores T e R
        DrawArrow3D((Vector3){0,0,0}, (Vector3){vT.x, vT.y, vT.z}, 0.05f, SKYBLUE);
        DrawArrow3D((Vector3){0,0,0}, (Vector3){vR.x, vR.y, vR.z}, 0.05f, ORANGE);

        // Vetor Up (referência +Z)
        DrawArrow3D((Vector3){0,0,0}, (Vector3){0,0,1.2f}, 0.05f, GREEN);

        // Arcos de azimute desde N (az=0) até AZ_T e AZ_R (no horizonte)
        DrawAzimuthArc(0.0f, AZ_T, Fade(SKYBLUE, 0.8f));
        DrawAzimuthArc(0.0f, AZ_R, Fade(ORANGE, 0.8f));

        // Arcos de elevação ao longo dos meridianos de T e R (de 0 até EL)
        DrawElevationArc(AZ_T, EL_T, Fade(SKYBLUE, 0.8f));
        DrawElevationArc(AZ_R, EL_R, Fade(ORANGE, 0.8f));

        // Arco do ângulo J entre T e R (grande círculo)
        DrawGreatCircleArc(vT, vR, YELLOW);

        // Rótulos projetados em 2D com marcadores esféricos
        Vector3 pT = Vector3Scale(vT, 1.05f);
        Vector3 pR = Vector3Scale(vR, 1.05f);
        DrawSphere(pT, 0.02f, SKYBLUE);
        DrawSphere(pR, 0.02f, ORANGE);
        Vector2 sT = GetWorldToScreen(pT, cam);
        Vector2 sR = GetWorldToScreen(pR, cam);
        DrawText("T", (int)sT.x + 6, (int)sT.y - 10, 18, RAYWHITE);
        DrawText("R", (int)sR.x + 6, (int)sR.y - 10, 18, RAYWHITE);

        // Rótulos N (AZ=0°) e E (AZ=90°) no equador
        Vector2 sN = GetWorldToScreen((Vector3){1.05f, 0.0f, 0.0f}, cam);
        Vector2 sE = GetWorldToScreen((Vector3){0.0f, 1.05f, 0.0f}, cam);
        DrawText("N (AZ=0°)", (int)sN.x + 6, (int)sN.y - 10, 16, RAYWHITE);
        DrawText("E (AZ=90°)", (int)sE.x + 6, (int)sE.y - 10, 16, RAYWHITE);

        // Rótulo Up próximo ao topo
        Vector2 sUp = GetWorldToScreen((Vector3){0.0f, 0.0f, 1.15f}, cam);
        DrawText("Up", (int)sUp.x + 6, (int)sUp.y - 10, 16, GREEN);

        // Rótulo 'j' do ângulo entre T e R (no ponto médio do arco)
        Vector3 mid = SlerpUnit(vT, vR, 0.5f);
        Vector2 sj = GetWorldToScreen(Vector3Scale(mid, 1.03f), cam);
        DrawText("j", (int)sj.x + 4, (int)sj.y - 10, 20, YELLOW);

        EndMode3D();

        // HUD
        const int pad = 12; int y = pad; const int line = 22;
        DrawRectangle(pad-6, pad-6, 520, 180, Fade(BLACK, 0.45f));
        DrawText("Trigonometria Esférica — Ângulo J", pad, y, 22, RAYWHITE); y += line + 4;
        DrawText(TextFormat("Alvo  T: Az=%.1f°, El=%.1f°", azT_deg, elT_deg), pad, y, 18, RAYWHITE); y += line;
        DrawText(TextFormat("Eixo  R: Az=%.1f°, El=%.1f°", azR_deg, elR_deg), pad, y, 18, RAYWHITE); y += line;
        DrawText(TextFormat("J(T,R) ≈ %.3f°  (verificação: %.3f°)", Jdeg, Jdeg_trig), pad, y, 18, YELLOW);

        DrawText("Controles: T(A/D,W/S), R(J/L,I/K), Reset(R), Mouse Orbita", pad, GetScreenHeight()-28, 18, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
