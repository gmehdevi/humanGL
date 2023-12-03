#include "humanGL.hpp"
#include "imgui.h"

void animationEditor(Bone *root)
{
	ImGui::Begin("Animation Editor");
	static string current_animation_name = "";
	static char new_animation_name[100] = "";
	static char load_animation_name[100] = "";
	static float time = 0.0f;

	if (ImGui::BeginCombo("Animations", current_animation_name.c_str()))
	{
		for (auto &anim : animations)
		{
			bool is_selected = (current_animation_name == anim.first);

			if (ImGui::Selectable(anim.first.c_str(), is_selected))
			{
				current_animation_name = anim.first;
				animation animation = animations[current_animation_name];

				if (animation.size() > 0)
					time = animation.rbegin()->first;
				else
					time = 0.0f;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	if (current_animation_name != "")
	{
		if (animations[current_animation_name].size() > 0)
			ImGui::SliderFloat("Time", &time, animations[current_animation_name].rbegin()->first, 10.0f);

		if (ImGui::Button("Save Keyframe"))
		{
			animations[current_animation_name].insert(std::make_pair(time, root->getTransforms()));
			std::cout << "Saved keyframe for time " << time << std::endl;
		}

		if (animations[current_animation_name].size() > 0 && ImGui::Button("Delete Last Keyframe"))
		{
			std::cout << "Deleted keyframe for time " << animations[current_animation_name].rbegin()->first << std::endl;
			animations[current_animation_name].erase(animations[current_animation_name].rbegin()->first);
		}

		if (ImGui::Button("Delete Animation"))
		{
			animations.erase(current_animation_name);
			current_animation_name = "";
		}

		if (animations[current_animation_name].size() > 1 && ImGui::Button("Save to file"))
			saveAnimation(current_animation_name, animations[current_animation_name]);
	}

	ImGui::Separator();

	ImGui::InputText("New", new_animation_name, 100);

	if (new_animation_name[0] != '\0')
	{
		if (ImGui::Button("Create Animation"))
		{
			animations[new_animation_name] = animation();

			if (current_animation_name != "")
			{
				if (animations[current_animation_name].size() > 0)
					time = animations[current_animation_name].rbegin()->first;
				else
					time = 0.0f;
			}

			std::cout << "Created new animation " << new_animation_name << std::endl;
			new_animation_name[0] = '\0';
		}
	}

	ImGui::InputText("Load", load_animation_name, 100);

	if (load_animation_name[0] != '\0')
	{
		if (ImGui::Button("Load Animation"))
		{
			animation animation = loadAnimation(load_animation_name);
			auto path = std::filesystem::path(load_animation_name);
			string name = path.stem();

			if (animation.size() > 0)
				animations[name] = animation;
			load_animation_name[0] = '\0';
		}
	}

	ImGui::Separator();

	ImGui::Text("Play Animation");
	for (auto &anim : animations)
	{
		if (anim.second.size() > 1 && ImGui::Button(anim.first.c_str()))
		{
			current_animation = &anim.second;
			start_time = std::chrono::high_resolution_clock::now();
			end_time = start_time + std::chrono::milliseconds((int)(anim.second.rbegin()->first * 1000));
			std::cout << "Playing animation " << anim.first << std::endl;
		}
	}

	ImGui::End();
}

void saveAnimation(const string name, const animation &a)
{
	std::ofstream file(name + ".anim");

	if (!file.is_open())
	{
		std::cerr << "Could not open file " << name << std::endl;
		return;
	}

	for (const auto &[time, transforms] : a)
	{
		file << time << "\n";
		for (const mat &m : transforms)
			file << m << "\n/";
		file << "~";
	}

	file.close();

	std::cout << "Animation " << name << " saved" << std::endl;
}

std::map<string, animation> loadAnimationsFromDir(string dir_path)
{
	std::map<string, animation> animations;
	for (const auto &entry : std::filesystem::directory_iterator(dir_path))
	{
		if (entry.path().extension() != ".anim")
			throw std::runtime_error("Invalid file type");

		auto path = entry.path();
		string name = path.stem();
		std::cout << "Loading animation " << name << std::endl;
		path = path.replace_extension("");
		animations[name] = loadAnimation(path);
	}

	return animations;
}

animation loadAnimation(const string name)
{
	std::ifstream file(name + ".anim");
	if (!file.is_open())
	{
		std::cerr << "Animation " << name << " not found" << std::endl;
		return animation();
	}

	std::stringstream buffer;

	buffer << file.rdbuf();

	animation a;
	std::vector<string> frames = split_set(buffer.str(), "~");
	for (string frame : frames)
	{
		float time = std::stof(frame.substr(0, frame.find_first_of("\n")));
		frame.erase(0, frame.find_first_of("\n") + 1);
		std::vector<string> transforms = split_set(frame, "/");
		std::vector<mat> mats;
		for (string transform : transforms)
		{
			mat m(4);
			std::stringstream ss(transform);
			ss >> m;
			mats.push_back(m);
		}
		a[time] = mats;
	}

	file.close();
	return a;
}

std::vector<string> split_set(string s, string delimiter)
{
	std::vector<string> ret;
	size_t pos_start = 0, pos_end = 0;
	string token;

	while ((pos_start = s.find_first_not_of(delimiter, pos_end)) != string::npos)
	{
		pos_start = s.find_first_not_of(delimiter, pos_end);
		pos_end = s.find_first_of(delimiter, pos_start);
		ret.push_back(s.substr(pos_start, pos_end - pos_start));
	}
	return ret;
}

void runAnimation(Bone *root, animation &a, std::chrono::_V2::system_clock::time_point start)
{
	float t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0f;
	auto before = a.lower_bound(t);
	auto after = a.upper_bound(t);

	if (before != a.begin())
		before--;
	if (after == a.end())
		after--;
	t = (t - before->first) / (after->first - before->first);

	std::vector<mat> transforms;
	for (int i = 0; i < before->second.size(); i++)
		transforms.push_back(linear_interpolation(before->second[i], after->second[i], t));

	root->setTransforms(transforms);
}

bool is_valid_animation_name(animation &a)
{
	if (a.size() < 2){
		return false;
	} else if (a.begin()->first != 0.0f) {
		return false;
	} else {
		int num_transforms = a.begin()->second.size();
		num_transforms = 1;
		for (auto &keyframe : a)
		{
			for (auto &transform : keyframe.second)
				if (transform.rows() != 4 || transform.cols() != 4)
					return false;
			if (keyframe.second.size() != num_transforms)
				return false;
		}
	}
	return true;
}