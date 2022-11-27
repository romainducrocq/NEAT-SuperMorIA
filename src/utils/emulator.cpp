#include "utils/emulator.hpp"

nintaco::Emulator::Emulator()
{
    this->rom.run();
    this->api.make();

    nintaco::Api::API(this->api.ptr());
    nintaco::Event::EVENT().init();
}

void nintaco::Emulator::app_run(void(*app_loop_func)(NintacoAPI*))
{
    this->app_setup();
    nintaco_addFrameListener(this->api.ptr(), app_loop_func);
    nintaco_addStatusListener(this->api.ptr(), [](NintacoAPI*, char* msg) { std::cout << msg << "\n"; } );
    nintaco_run(this->api.ptr());
}

void nintaco::Emulator::app_setup()
{
    this->ev_setup();
    this->setup();
    this->draw_setup();
}

void nintaco::Emulator::app_loop()
{
    if(this->api.ready()){
        if(! nintaco::Event::EVENT().open()){
            this->api.kill();
            return;
        }
        nintaco::Event::EVENT().process();

        if(! this->loop()){
            this->api.kill();
            return;
        }
        this->draw_loop();
    }
}

void nintaco::State::load()
{
    nintaco_loadState(nintaco::Api::API(), &this->sav[0]);
}

void nintaco::Event::init()
{
    this->window = std::make_unique<sf::RenderWindow>();
    this->ev_manager = std::make_unique<sfev::EventManager>(*this->window, true);

    sf::ContextSettings options;
    options.antialiasingLevel = 0;

    this->window->create(sf::VideoMode(1, 1, 1), "Nintaco", sf::Style::Default, options);
    this->window->setVerticalSyncEnabled(false);

    this->window->setFramerateLimit(0);
}

bool nintaco::Event::open()
{
    return this->window->isOpen();
}

void nintaco::Event::process()
{
    this->ev_manager->processEvents();
}

void nintaco::Event::ev_setup()
{
    for(const auto& action : this->actions){
        this->ev_manager->addKeyPressedCallback(this->ev_key.at(action), [&](sfev::CstEv){
            this->ev_state.at(action) = true;
        });

        this->ev_manager->addKeyReleasedCallback(this->ev_key.at(action), [&](sfev::CstEv){
            this->ev_state.at(action) = false;
        });
    }
}

void nintaco::Event::get_action(std::vector<float>& act)
{
    assert(act.size() == this->actions.size());

    for(size_t i = 0; i < act.size(); i++){
        act[i] = this->ev_state.at(this->actions[i]) ? 1.f : -1.f;
    }
}

const sfev::EventManager& nintaco::Event::get_ev_manager() const { return *this->ev_manager; }
