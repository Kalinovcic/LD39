#pragma once

// meshes
Mesh penguin;

// sounds
Sound sound_fireball;

void load_data()
{
    create_mesh("data/models/robot.obj", &penguin);

    load_sound(&sound_fireball, "data/audio/fireball.wav");
}
