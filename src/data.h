#pragma once

// meshes
Mesh block;
Mesh robot;

// sounds
Sound sound_robot_move;
Sound sound_robot_turn;
Sound sound_robot_hit;

void load_data()
{
    create_mesh("data/models/block.obj", &block);
    create_mesh("data/models/robot.obj", &robot);

    load_sound(&sound_robot_move, "data/audio/robot_move.wav");
    load_sound(&sound_robot_turn, "data/audio/robot_turn.wav");
    load_sound(&sound_robot_hit,  "data/audio/robot_hit.wav");
}
