#pragma once

// meshes
Mesh mesh_block;
Mesh mesh_robot;
Mesh mesh_pickup;

// sounds
Sound sound_robot_move[3];
Sound sound_robot_turn;
Sound sound_robot_hit;

int next_move_sound;

void load_data()
{
    create_mesh("data/models/block.obj",  &mesh_block);
    create_mesh("data/models/robot.obj",  &mesh_robot);
    create_mesh("data/models/pickup.obj", &mesh_pickup);

    load_sound(&sound_robot_move[0], "data/audio/robot_move0.wav");
    load_sound(&sound_robot_move[1], "data/audio/robot_move1.wav");
    load_sound(&sound_robot_move[2], "data/audio/robot_move2.wav");
    load_sound(&sound_robot_turn,    "data/audio/robot_turn.wav");
    load_sound(&sound_robot_hit,     "data/audio/robot_hit.wav");
}
