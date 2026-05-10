/*
 * main.cpp
 */

#include "app/Window.h"
#include "app/Loop.h"
#include "render/passes/WorldAxis.h"
#include "render/passes/Grid.h"
#include "render/passes/Lines.h"
#include "render/passes/Fog.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "scene/Scene.h"
#include "scene/Model.h"
#include "scene/Camera.h"
#include "scene/Light.h"

using namespace BulletRender;

int main()
{
    // window
    app::WindowConfig windowCfg{800, 600, "Demo", true, true};
    if (!app::Window::init(windowCfg))
    {
        return -1;
    }

    // renderer
    render::RenderConfig renderCfg{{0.05f, 0.05f, 0.08f, 1.0f}};
    render::Renderer::init(renderCfg);

    // grid
    auto grid = std::make_shared<render::Grid>();
    render::Renderer::registerPrePass(grid);

    // world coordinated
    auto worldAxis = std::make_shared<render::WorldAxis>();
    render::Renderer::registerPrePass(worldAxis);

    // lines
    auto lines = std::make_shared<render::Lines>();
    render::Renderer::registerPrePass(lines);

    // assets
    scene::Model model("assets/models/fox.obj");
    std::shared_ptr<render::GraphicsShader> shader = std::make_shared<render::GraphicsShader>(
        "assets/shaders/normal.vert.glsl",
        "assets/shaders/normal.frag.glsl"
    );

    // scene
    scene::Scene scene;

    scene::FlyCamera camera({0, 1, 5});
    scene.setCamera(&camera);

    scene::DirectionalLight light;
    scene.setLight(&light);

    // fog
    auto fog = std::make_shared<render::Fog>(true, 10.0f, 90.0f);
    render::Renderer::registerPostPass(fog);

    scene::SceneObject* object = scene.addObject(&model);
    object->getMaterial().setShader(shader);
    object->getMaterial().setColor({1.0f, 0.5f, 0.0f});
    object->getTransform().setPosition({0, 0, 0});

    // loop
    app::Loop loop(scene);
    loop.run(
        [&](float dt) {
            camera.update(app::Window::get(), dt);
            object->getTransform().rotateZ(0.5f * dt);
            lines->setThickness(5.0f);
            lines->addLine({-2.0f, 0.0f, -5.0f}, { 7.0f, 6.0f, -8.0f}, {1.0f, 1.0f, 1.0f});
        }
    );

    app::Window::shutdown();
    return 0;
}