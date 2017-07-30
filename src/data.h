#pragma once

// meshes
Mesh mesh_block;
Mesh mesh_robot;
Mesh mesh_box;
Mesh mesh_long_box;
Mesh mesh_detector;
Mesh mesh_splitter;
Mesh mesh_merger;

// sounds
Sound sound_robot_move[3];
Sound sound_robot_turn;
Sound sound_robot_hit;
Sound sound_detector_on;
Sound sound_detector_off;
Sound sound_poof;

// textures
GLuint texture_battery;

// fonts
Font font;

int next_move_sound;

void load_data()
{
    create_mesh("data/models/block.obj",    &mesh_block);
    create_mesh("data/models/robot.obj",    &mesh_robot);
    create_mesh("data/models/box.obj",      &mesh_box);
    create_mesh("data/models/longbox.obj",  &mesh_long_box);
    create_mesh("data/models/detector.obj", &mesh_detector);
    create_mesh("data/models/splitter.obj", &mesh_splitter);
    create_mesh("data/models/merger.obj",   &mesh_merger);

    load_sound(&sound_robot_move[0], "data/audio/robot_move0.wav");
    load_sound(&sound_robot_move[1], "data/audio/robot_move1.wav");
    load_sound(&sound_robot_move[2], "data/audio/robot_move2.wav");
    load_sound(&sound_robot_turn,    "data/audio/robot_turn.wav");
    load_sound(&sound_robot_hit,     "data/audio/robot_hit.wav");
    load_sound(&sound_detector_on,   "data/audio/detector_on.wav");
    load_sound(&sound_detector_off,  "data/audio/detector_off.wav");
    load_sound(&sound_poof,          "data/audio/poof.wav");

    texture_battery = load_texture("data/textures/battery.png");

    load_font("data/fonts/NotoSans-Regular.ttf", &font);
}
