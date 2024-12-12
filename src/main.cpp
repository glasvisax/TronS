#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#define RENDER_PREF
#define GAME_PREF
#define NETWORK_PREF
#include "misc/game_preferences.h"

#include "world/game.h"
#include "shaders/shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

inline void init_gl_buffers();
inline void init_glfw_window();
inline void init_shader();

inline void render_game();
inline void render_main_menu();
inline void render_lobby();
inline void render_client_connection_info();
inline void render_game_over();
inline void render_preloader();
inline void render_server_game_params();

void main_menu_start_server_cb();
void main_menu_start_client_cb();
void client_connection_info_start_cb();
void lobby_start_cb();
void preloader_stop_searching_cb();
void on_game_over_exit_cb();
void on_game_over_return_lobby_cb();

void on_lobby_return_menu_cb();
void on_connected_cb();
void on_disconnected_cb();
void on_game_over_cb(GameResult result);
void on_client_received_start_cb();

constexpr float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);

Game* gamePtr = nullptr;
GLFWwindow* window = nullptr;

Shader shader;
unsigned int VBO, VAO;

bool isServer = false;

char game_buf[sizeof(Game)];
char address_buf[20];
char port_buf[6];

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int current_width = SCR_WIDTH;
int current_height = SCR_HEIGHT;

int current_field_sizeX = 20;
int current_field_sizeZ = 20;

GameResult current_result;

enum RenderState : uint8_t 
{
    MAIN_MENU,
    CLIENT_CONNECTION_INFO,
    CONNECTING_PRELOADER,
    LOBBY, 
    GAME_ACTIVE,
    GAME_OVER
} State;

struct Player {
    bool isConnected;
    const char* name;
} player1{ true, "player1" }, player2{ false, "player2" };

bool disc = false;
int main(int argc, char** argv)
{
    memcpy(address_buf, default_address, sizeof(default_address) + 1);
    memcpy(port_buf, default_port, sizeof(default_port) + 1);

    State = RenderState::MAIN_MENU;

    init_glfw_window();
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    init_gl_buffers();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (State)
        {
            case MAIN_MENU: 
            {
                //if(gamePtr) gamePtr->shutDownConnection();
                render_main_menu();

                break;
            }
            case CLIENT_CONNECTION_INFO:
            { 
                render_client_connection_info();

                break; 
            }
            case CONNECTING_PRELOADER:
            {
                render_preloader();
                float currentFrame = static_cast<float>(glfwGetTime());
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;
                gamePtr->Update(deltaTime);
                break;
            }
            case LOBBY:
            { 
                render_lobby();
                float currentFrame = static_cast<float>(glfwGetTime());
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;
                gamePtr->Update(deltaTime);
                break;
            }
            case GAME_ACTIVE:
            { 
                float currentFrame = static_cast<float>(glfwGetTime());
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;
                gamePtr->Update(deltaTime);

                if (gamePtr->getState() != GameState::Active && !lastRender) {
                    break;
                }

                render_game();

                break;
            }
            case GAME_OVER:
            {
                render_game_over();

                float currentFrame = static_cast<float>(glfwGetTime());
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;
                gamePtr->Update(deltaTime);

                break; 
            }
            default:
            {
                break; 
            }
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    if(gamePtr) 
    {
        gamePtr->~Game();
    }
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    current_width = width;
    current_height = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        gamePtr->ProcessInput(key);
    }
}

inline void init_gl_buffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

inline void init_glfw_window() 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Snake", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        assert(0);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
}

inline void init_shader()
{
    std::filesystem::path _path = std::filesystem::current_path();
    std::string current_path = _path.string();
    std::string vetex_shader_path = current_path + vertex_shader_filename;
    std::string fragment_shader_path = current_path + fragment_shader_filename;

    shader.setVertex(vetex_shader_path.c_str());
    shader.setFragment(fragment_shader_path.c_str());
    shader.linkProgram();
    shader.use();

    shader.setVec3("lightPos", lightPos);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("viewPos", gamePtr->GetCamera().GetPosition());
    float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
    glm::mat4 projection = gamePtr->GetCamera().GetProjectionMatrix(aspect);
    glm::mat4 view = gamePtr->GetCamera().GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
}

inline void render_game()
{
    shader.use();

    shader.setVec3("objectColor", snake1Color);
    for (const auto& _part : gamePtr->GetSnake().GetBodyParts()) {
        glm::vec3 part(_part.x, 0.0f, _part.y);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, part);
        model = glm::scale(model, glm::vec3(0.9f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    shader.setVec3("objectColor", snake2Color);
    for (const auto& _part : gamePtr->GetSnake2().GetBodyParts()) {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 part(_part.x, 0.0f, _part.y);
        model = glm::translate(model, part);
        model = glm::scale(model, glm::vec3(0.9f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    shader.setVec3("objectColor", appleColor);
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec2 xy_pos = gamePtr->GetApplePosition();
    glm::vec3 part(xy_pos.x, 0.0f, xy_pos.y);
    model = glm::translate(model, part);
    model = glm::scale(model, glm::vec3(0.9f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.setVec3("objectColor", borderColor);
    glm::vec2 gridSize = gamePtr->GetGridSize();
    for (int x = -1; x <= gridSize.x; x += static_cast<int>(gridSize.y + 1)) {
        for (int z = -1; z <= gridSize.y; z += static_cast<int>(gridSize.y + 1)) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.0f, z));
            model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    for (int x = -1; x <= gridSize.x; x += static_cast<int>(gridSize.x + 1)) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, 0.0f, (gridSize.y - 1) / 2.0f));
        model = glm::scale(model, glm::vec3(0.25f, 0.25f, gridSize.y + 1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    for (int z = -1; z <= gridSize.y; z += static_cast<int>(gridSize.y + 1)) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((gridSize.x - 1) / 2.0f, 0.0f, z));
        model = glm::scale(model, glm::vec3(gridSize.x + 1.0f, 0.25f, 0.25f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    for (int i = 0; i < gridSize.x; ++i) {
        for (int j = 0; j < gridSize.y; ++j) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, -1.0f, j));
            model = glm::scale(model, glm::vec3(1.0f));
            shader.setMat4("model", model);
            float diff = float(i + j) * 0.04f;
            glm::vec3 current_color(diff, diff, diff);
            shader.setVec3("objectColor", current_color);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

inline void render_main_menu()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float saved_current_width = 0.0f;
    static float saved_current_height = 0.0f;
    static float menu_width = static_cast<float>(current_width) / 4.0f;
    static float menu_height = static_cast<float>(current_height) / 3.0f;
    static float menu_pos_x;
    static float menu_pos_y;
    static float button_width = menu_width - 20.0f;
    static float button_height = menu_height / 5.0f;
    static float dummy_y;
    
    if(saved_current_width != current_width)
    {
        saved_current_width = static_cast<float>(current_width);
        menu_pos_x = (saved_current_width - menu_width) / 2;
    }
    if(saved_current_height != current_height)
    {
        saved_current_height = static_cast<float>(current_height);
        menu_pos_y = (saved_current_height - menu_width) / 2;
        dummy_y = (menu_height - 3 * button_height) / 8.0f;
    }
    ImVec2 menu_size(menu_width, menu_height);
    ImVec2 menu_pos(menu_pos_x, menu_pos_y);

    ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menu_size);
    ImGui::Begin("Main Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.0f, dummy_y));
    if (ImGui::Button("Start server", ImVec2(button_width, button_height))) {
        main_menu_start_server_cb();
    }

    ImGui::Dummy(ImVec2(0.0f, dummy_y));
    if (ImGui::Button("Start client", ImVec2(button_width, button_height))) {
        main_menu_start_client_cb();
    }

    ImGui::Dummy(ImVec2(0.0f, dummy_y));
    if (ImGui::Button("Exit", ImVec2(button_width, button_height))) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
 
inline void render_client_connection_info()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float saved_current_width = 0.0f;
    static float saved_current_height = 0.0f;
    static float menu_width = static_cast<float>(current_width) / 4.0f;
    static float menu_height = static_cast<float>(current_height) / 3.0f;
    static float menu_pos_x;
    static float menu_pos_y;
    static float button_width = menu_width - 20.0f;
    static float button_height = menu_height / 5.0f;
    static float dummy_y;

    if (saved_current_width != current_width)
    {
        saved_current_width = static_cast<float>(current_width);
        menu_pos_x = (saved_current_width - menu_width) / 2;
    }
    if (saved_current_height != current_height)
    {
        saved_current_height = static_cast<float>(current_height);
        menu_pos_y = (saved_current_height - menu_width) / 2;
        dummy_y = (menu_height - 3 * button_height) / 8.0f;
    }

    ImVec2 menu_size(menu_width, menu_height);
    ImVec2 menu_pos(menu_pos_x, menu_pos_y);

    ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menu_size);
    ImGui::Begin("Start Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.0f, dummy_y));

    ImGui::Text("Enter text:");
    ImGui::InputText("##ip_address", address_buf, IM_ARRAYSIZE(address_buf));
    ImGui::Spacing();
    ImGui::InputText("##port", port_buf, IM_ARRAYSIZE(port_buf));

    ImGui::Spacing();

    if (ImGui::Button("Start", ImVec2(button_width, button_height))) {
        client_connection_info_start_cb();
    }

    ImGui::Spacing();
    if (ImGui::Button("Main Menu", ImVec2(button_width, button_height))) {
        State = RenderState::MAIN_MENU;
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void render_lobby() 
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float saved_current_width = 0.0f;
    static float saved_current_height = 0.0f;

    static float menu_width = static_cast<float>(current_width) / 3.0f;
    static float menu_height = static_cast<float>(current_height) / 3.0f;

    static float param_menu_width = static_cast<float>(current_width) / 3.0f;
    static float param_menu_height = static_cast<float>(current_height) / 4.0f;

    static float menu_pos_x;
    static float menu_pos_y;
    static float param_menu_pos_x;
    static float param_menu_pos_y;
    static float button_width = menu_width - 20.0f;
    static float button_height = menu_height / 5.0f;
    static float dummy_y;

    if (saved_current_width != current_width)
    {
        saved_current_width = static_cast<float>(current_width);
        menu_pos_x = (saved_current_width - menu_width) / 2;
        param_menu_pos_x = menu_pos_x;
    }
    if (saved_current_height != current_height)
    {
        saved_current_height = static_cast<float>(current_height);
        menu_pos_y = (saved_current_height - menu_height) / 2;
        param_menu_pos_y = (menu_pos_y - menu_height);
        dummy_y = (menu_height - 3 * button_height) / 8.0f;
    }

    ImVec2 menu_size(menu_width, menu_height);
    ImVec2 menu_pos(menu_pos_x, menu_pos_y);

    ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menu_size);

    ImGui::Begin("Lobby", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.0f, dummy_y));

    ImGui::Text("Room Setup");

    ImGui::Spacing();

    ImGui::Text("Server Address: %s", address_buf);
    ImGui::Text("Port: %s", port_buf);

    ImGui::Spacing();

    ImGui::Text("%s: %s", player1.name, player1.isConnected ? "Connected" : "Disconnected");
    ImGui::SameLine();
    if (player1.isConnected) {
        ImGui::Text("(+)");
    }
    else {
        ImGui::Text("(-)");
    }

    ImGui::Spacing();

    ImGui::Text("%s: %s", player2.name, player2.isConnected ? "Connected" : "Disconnected");
    ImGui::SameLine();
    if (player2.isConnected) {
        ImGui::Text("(+)");
    }
    else {
        ImGui::Text("(-)");
    }

    ImGui::Spacing();

    if (ImGui::Button("Main Menu", ImVec2(button_width, button_height))) {
        on_lobby_return_menu_cb();
    }

    if (isServer) {
        bool canStartGame = player1.isConnected && player2.isConnected;

        if (!canStartGame) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }

        ImVec4 buttonColor = canStartGame ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Зеленая для активной, серо-серая для неактивной
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        
        if (ImGui::Button("Start Game", ImVec2(button_width, button_height))) {
            lobby_start_cb();
        }

        ImGui::PopStyleColor();

        if (!canStartGame) {
            ImGui::PopItemFlag(); 
        }
        ImGui::Spacing();

        ImGui::End();
        {
            ImVec2 menuSize(param_menu_width, param_menu_height);
            ImVec2 menuPos(param_menu_pos_x, param_menu_pos_y);

            static char field_size_x_buf[3] = "20";
            static char field_size_z_buf[3] = "20";

            ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(menuSize);
            ImGui::Begin("Game Parameters", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Field size x:");
            ImGui::InputText("##param1", field_size_x_buf, 3);

            ImGui::Spacing();
            ImGui::Text("Field size z:");
            ImGui::InputText("##param2", field_size_z_buf, 3);

            int field_size_x;
            if (strlen(field_size_x_buf) > 0) {
                sscanf_s(field_size_x_buf, "%d", &field_size_x);
                if (field_size_x < 4)
                {
                    sprintf_s(field_size_x_buf, "%d", 4);
                    current_field_sizeX = 4;
                }
                else if (field_size_x > 40)
                {
                    sprintf_s(field_size_x_buf, "%d", 40);
                    current_field_sizeX = 40;
                }
                current_field_sizeX = field_size_x;
            } else 
            {
                field_size_x_buf[0] = 2;
                field_size_x_buf[1] = 0;
                current_field_sizeX = 20;
            }

            
            int field_size_z;
            if (strlen(field_size_z_buf) > 0) {
                sscanf_s(field_size_z_buf, "%d", &field_size_z);
                if (field_size_z < 4)
                {
                    sprintf_s(field_size_z_buf, "%d", 4);
                    current_field_sizeZ = 4;
                } else if (field_size_z > 40)
                {
                    sprintf_s(field_size_z_buf, "%d", 40);
                    current_field_sizeZ = 40;
                }
                current_field_sizeZ = field_size_z;
            }
            else
            {
                field_size_z_buf[0] = 2;
                field_size_z_buf[1] = 0;
                current_field_sizeZ = 20;
            }

        }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void render_game_over()
{
    std::string result;
    if(current_result == GameResult::Tie)
    {
        result = "It's Tie!";
    }
    else if(current_result == GameResult::Snake1)
    {
        result = "Player1 Win!";
    }
    else 
    {
        result = "Player2 Win!";
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float saved_current_width = 0.0f;
    static float saved_current_height = 0.0f;
    static float menu_width = static_cast<float>(current_width) / 4.0f;
    static float menu_height = static_cast<float>(current_height) / 3.0f;
    static float menu_pos_x;
    static float menu_pos_y;
    static float button_width = menu_width - 20.0f;
    static float button_height = menu_height / 5.0f;
    static float dummy_y;

    if (saved_current_width != current_width)
    {
        saved_current_width = static_cast<float>(current_width);
        menu_pos_x = (saved_current_width - menu_width) / 2;
    }
    if (saved_current_height != current_height)
    {
        saved_current_height = static_cast<float>(current_height);
        menu_pos_y = (saved_current_height - menu_width) / 2;
        dummy_y = (menu_height - 3 * button_height) / 8.0f;
    }

    ImVec2 menu_size(menu_width, menu_height);
    ImVec2 menu_pos(menu_pos_x, menu_pos_y);

    ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menu_size);
    ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.0f, dummy_y));

    ImGui::Text("Game Over");

    ImGui::Spacing();

    ImGui::Text("Result: %s", result.c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Exit to Menu", ImVec2(button_width, button_height))) {
        on_game_over_exit_cb();
    }

    ImGui::Spacing();


    if (ImGui::Button("Return to Lobby", ImVec2(button_width, button_height))) {
        on_game_over_return_lobby_cb();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void render_preloader()
{
    static float loadingProgress = 0.0f;
    static float loadingSpeed = 0.0001f; 

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float saved_current_width = 0.0f;
    static float saved_current_height = 0.0f;
    static float menu_width = static_cast<float>(current_width) / 4.0f;
    static float menu_height = static_cast<float>(current_height) / 3.0f;
    static float menu_pos_x;
    static float menu_pos_y;
    static float button_width = menu_width - 20.0f;
    static float button_height = menu_height / 5.0f;
    static float dummy_y;

    if (saved_current_width != current_width)
    {
        saved_current_width = static_cast<float>(current_width);
        menu_pos_x = (saved_current_width - menu_width) / 2;
    }
    if (saved_current_height != current_height)
    {
        saved_current_height = static_cast<float>(current_height);
        menu_pos_y = (saved_current_height - menu_width) / 2;
        dummy_y = (menu_height - 3 * button_height) / 8.0f;
    }

    ImVec2 menu_size(menu_width, menu_height);
    ImVec2 menu_pos(menu_pos_x, menu_pos_y);

    ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(menu_size);
    ImGui::Begin("Preloader", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImVec2(0.0f, dummy_y));
    ImGui::Text("Searching for players...");

    ImGui::Spacing();

    loadingProgress += loadingSpeed;
    if (loadingProgress > 1.0f) loadingProgress = 0.0f;

    ImGui::ProgressBar(loadingProgress, ImVec2(button_width, button_height), NULL);

    ImGui::Spacing();

    if (ImGui::Button("Stop Searching", ImVec2(button_width, button_height))) {
        preloader_stop_searching_cb();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void main_menu_start_server_cb()
{
    player2.isConnected = false;
    int port;
    sscanf_s(port_buf, "%d", &port);
    int init_port = port;
    if (!gamePtr)
    {
        new (game_buf) Game(current_field_sizeX, current_field_sizeX);
        gamePtr = reinterpret_cast<Game*>(game_buf);
        init_shader();
    } else 
    {
        gamePtr->shutDownConnection();
    }

    isServer = true;
    gamePtr->initializeServer(port);
    if(init_port != port)
    {
        sprintf_s(port_buf, "%d", port);
    }
    gamePtr->onConnected = on_connected_cb;
    gamePtr->onGameOver = on_game_over_cb;
    gamePtr->onDisconnected = on_disconnected_cb;
    State = RenderState::LOBBY;
}

void main_menu_start_client_cb()
{
    isServer = false;
    if (!gamePtr)
    {
        new (game_buf) Game(current_field_sizeX, current_field_sizeX);
        gamePtr = reinterpret_cast<Game*>(game_buf);
        init_shader();
    } else
    {
        gamePtr->shutDownConnection();
    }

    State = RenderState::CLIENT_CONNECTION_INFO;
}

void client_connection_info_start_cb() 
{
    int port;
    sscanf_s(port_buf, "%d", &port);
    gamePtr->initializeClient(port, address_buf);
    gamePtr->onConnected = on_connected_cb;
    gamePtr->onDisconnected = on_disconnected_cb;
    gamePtr->onGameOver = on_game_over_cb;
    gamePtr->onClientReceivedStart = on_client_received_start_cb;
    State = RenderState::CONNECTING_PRELOADER;
}

void lobby_start_cb()
{
    gamePtr->SetGridSize(current_field_sizeX, current_field_sizeZ);
    init_shader();
    gamePtr->ServerGameStart();
    State = RenderState::GAME_ACTIVE;
}

void on_connected_cb()
{
    player2.isConnected = true;
    State = RenderState::LOBBY;
}

void on_disconnected_cb()
{
    player2.isConnected = false;
    if(!isServer)
    {
        State = RenderState::MAIN_MENU;
    } else 
    {
        State = RenderState::LOBBY;
    }
}

void on_game_over_cb(GameResult result)
{
    State = RenderState::GAME_OVER;

    current_result = result;
}

void preloader_stop_searching_cb() 
{
    State = RenderState::MAIN_MENU;
}

void on_client_received_start_cb()
{
    init_shader();
    State = RenderState::GAME_ACTIVE;
}

void on_game_over_return_lobby_cb()
{
    State = RenderState::LOBBY;
}

void on_game_over_exit_cb() 
{
    //gamePtr->shutDownConnection();
    State = RenderState::MAIN_MENU;
}

void on_lobby_return_menu_cb()
{
    //gamePtr->shutDownConnection();

    State = RenderState::MAIN_MENU;
}