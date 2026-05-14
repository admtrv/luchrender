/*
 * main.cpp
 */

#include "app/Window.h"
#include "app/Loop.h"
#include "render/passes/WorldAxis.h"
#include "render/passes/Grid.h"
#include "render/passes/Lines.h"
#include "render/passes/Fog.h"
#include "render/passes/SkyBox.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "render/textures/TextureLoader.h"
#include "render/textures/CubeMap.h"
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
    render::RenderConfig renderCfg{{0.0f, 0.0f, 0.0f, 1.0f}};
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
    scene::Model foxModel("assets/models/fox/fox.obj");
    scene::Model backpackModel("assets/models/backpack/backpack.obj");
    scene::Model teapotModel("assets/models/teapot/teapot.obj");
    std::shared_ptr<render::GraphicsShader> shader = std::make_shared<render::GraphicsShader>(
        "assets/shaders/normal.vert.glsl",
        "assets/shaders/normal.frag.glsl"
    );

/*
    std::shared_ptr<render::CubeMap> sky = std::make_shared<render::CubeMap>(std::array<std::string, 6>{
        "assets/textures/skybox/right.jpg",
        "assets/textures/skybox/left.jpg",
        "assets/textures/skybox/top.jpg",
        "assets/textures/skybox/bottom.jpg",
        "assets/textures/skybox/front.jpg",
        "assets/textures/skybox/back.jpg"
    });

    // skybox
    auto skybox = std::make_shared<render::SkyBox>(sky);
    render::Renderer::registerPrePass(skybox);

    // or
*/
    // fog
    auto fog = std::make_shared<render::Fog>(true, 10.0f, 90.0f);
    render::Renderer::registerPostPass(fog);

    // scene
    scene::Scene scene;

    scene::FlyCamera camera({0, 1, 5});
    scene.setCamera(&camera);

    // lights:

    // global light
    scene::AmbientLight ambient;
    ambient.setColor({0.4f, 0.45f, 0.55f});
    ambient.setIntensity(0.3f);
    scene.addLight(&ambient);

    // sun
    scene::DirectionalLight sun({-0.7f, 0.7f, 0.25f});
    sun.setColor({1.0f, 0.95f, 0.85f});
    sun.setIntensity(0.8f);
    scene.addLight(&sun);

    // lamp
    scene::SpotLight spot({-5.0f, 6.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, 15.0f, 25.0f, 15.0f);
    spot.setColor({1.0f, 1.0f, 1.0f});
    spot.setIntensity(2.0f);
    scene.addLight(&spot);

    // blue point
    scene::PointLight pointBlue({7.0f, 2.5f, 2.0f}, 8.0f);
    pointBlue.setColor({0.2f, 0.4f, 1.0f});
    pointBlue.setIntensity(4.0f);
    scene.addLight(&pointBlue);

    // pink point
    scene::PointLight pointPink({10.0f, 2.5f, -2.0f}, 8.0f);
    pointPink.setColor({1.0f, 0.3f, 0.7f});
    pointPink.setIntensity(4.0f);
    scene.addLight(&pointPink);

    // objects:

    // floor
    scene::Box floorModel(40.0f, 0.2f, 20.0f);
    scene::SceneObject* floor = scene.addObject(&floorModel);
    floor->getMaterial().setShader(shader);
    floor->getMaterial().setColor({0.5f, 0.5f, 0.55f});
    floor->getTransform().setPosition({3.5f, -0.15f, 0.0f});

    // fox with automatic colors from mtl
    scene::SceneObject* foxMtl = scene.addObject(&foxModel);
    foxMtl->getMaterial().setShader(shader);
    foxMtl->getTransform().setPosition({-5.0f, 0.0f, 0.0f});

    // fox with color
    scene::SceneObject* foxColor = scene.addObject(&foxModel);
    foxColor->getMaterial().setShader(shader);
    foxColor->getTransform().setPosition({0.0f, 0.0f, 0.0f});
    foxColor->getMaterial().setColor({1.0f, 0.5f, 0.0f});

    // backpack with automatic textures
    scene::SceneObject* backpackTexture = scene.addObject(&backpackModel);
    backpackTexture->getMaterial().setShader(shader);
    backpackTexture->getTransform().setPosition({5.0f, 1.5f, 0.0f});

    // teapot with texture
    auto metalTexture = render::TextureLoader::instance().load("assets/textures/metal.jpg");
    scene::SceneObject* teapotFileTexture = scene.addObject(&teapotModel);
    teapotFileTexture->getMaterial().setShader(shader);
    teapotFileTexture->getMaterial().setTexture("uAlbedo", metalTexture, 0);
    teapotFileTexture->getTransform().setPosition({12.0f, 1.5f, 0.0f});
    teapotFileTexture->getTransform().setScale({0.2f, 0.2f, 0.2f});

    // loop
    app::Loop loop(scene);
    loop.run(
        [&](float dt) {
            camera.update(app::Window::get(), dt);
            lines->setThickness(5.0f);
            lines->addLine({-5.0f, 5.0f, -5.0f}, {5.0f, 5.0f, -5.0f}, {1.0f, 1.0f, 1.0f});
        }
    );

    app::Window::shutdown();
    return 0;
}