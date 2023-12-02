#include "humanGL.hpp"
#include "imgui.h"

void animationEditor(Bone *root)
{
	static string current_animation_name = string();
	static float time = 0.0f;

	ImGui::Begin("Animation Editor");

	animationSelectionEditor(current_animation_name, time);

	if (!current_animation_name.empty())
		currentAnimationEditor(root, current_animation_name, time);

	ImGui::Separator();

	animationCreationEditor(current_animation_name, time);
	animationLoadEditor();

	ImGui::Separator();

	animationPlayEditor();

	ImGui::End();
}

void animationSelectionEditor(string &current_animation_name, float &time)
{
	ImGui::BeginDisabled(name_to_animations.empty());

	if (ImGui::BeginCombo("Animations", current_animation_name.c_str()))
	{
		for (auto &anim : name_to_animations)
		{
			bool is_selected = (current_animation_name == anim.first);

			if (ImGui::Selectable(anim.first.c_str(), is_selected))
			{
				current_animation_name = anim.first;
				setTimeToLastKeyframe(time, current_animation_name);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::EndDisabled();
}

void currentAnimationEditor(Bone *root, string &current_animation_name, float &time)
{
	bool current_animation_has_keyframes = !name_to_animations[current_animation_name].empty();
	bool current_animation_has_multiple_keyframes = name_to_animations[current_animation_name].size() > 1;
	float current_animation_last_time = current_animation_has_keyframes ? name_to_animations[current_animation_name].rbegin()->first
																		: 0.0f;

	ImGui::BeginDisabled(!current_animation_has_keyframes);
	ImGui::SliderFloat("Time", &time, current_animation_last_time, 10.0f);
	ImGui::EndDisabled();

	if (ImGui::Button("Save Keyframe"))
	{
		name_to_animations[current_animation_name].insert(std::make_pair(time, root->getAnimations()));
		std::cout << "Saved keyframe for time " << time << std::endl;
	}

	ImGui::BeginDisabled(!current_animation_has_keyframes);
	if (ImGui::Button("Delete Last Keyframe"))
	{
		std::cout << "Deleted keyframe for time " << current_animation_last_time << std::endl;
		name_to_animations[current_animation_name].erase(current_animation_last_time);
		setTimeToLastKeyframe(time, current_animation_name);
	}
	ImGui::EndDisabled();

	if (ImGui::Button("Delete Animation"))
	{
		name_to_animations.erase(current_animation_name);
		current_animation_name = string();

		return;
	}

	ImGui::BeginDisabled(!current_animation_has_multiple_keyframes);
	if (ImGui::Button("Save to file"))
		saveAnimations(current_animation_name, name_to_animations[current_animation_name]);
	ImGui::EndDisabled();
}

void saveAnimations(const string name, const Animations &animations)
{
	std::ofstream file(name + ".anim");

	if (!file.is_open())
	{
		std::cerr << "Could not open file " << name << std::endl;
		return;
	}

	for (const auto &[time, animation] : animations)
	{
		file << time << "\n";
		for (const auto &a : animation)
			file << a << "\n";
		file << "~";
	}

	file.close();

	std::cout << "Animation " << name << " saved" << std::endl;
}

void animationCreationEditor(string &current_animation_name, float &time)
{
	static char new_animation_name[100] = "";

	ImGui::InputText("New", new_animation_name, 100);

	if (new_animation_name[0] != '\0')
	{
		if (ImGui::Button("Create Animation"))
		{
			name_to_animations[new_animation_name] = Animations();

			if (!current_animation_name.empty())
				setTimeToLastKeyframe(time, current_animation_name);

			std::cout << "Created new animation " << new_animation_name << std::endl;
			new_animation_name[0] = '\0';
		}
	}
}

void animationLoadEditor()
{
	static char load_animation_name[100] = "";

	ImGui::InputText("Load", load_animation_name, 100);

	if (load_animation_name[0] != '\0')
	{
		if (ImGui::Button("Load Animation"))
		{
			Animations animations = loadAnimations(load_animation_name);

			if (!animations.empty())
			{
				auto path = std::filesystem::path(load_animation_name);
				string name = path.stem();

				name_to_animations[name] = animations;
			}
			load_animation_name[0] = '\0';
		}
	}
}

void animationPlayEditor()
{
	ImGui::Text("Play Animation");

	for (auto &anim : name_to_animations)
	{
		if (anim.second.size() > 1 && ImGui::Button(anim.first.c_str()))
		{
			current_animation = &anim.second;
			start_time = std::chrono::high_resolution_clock::now();
			end_time = start_time + std::chrono::milliseconds((int)(anim.second.rbegin()->first * 1000));
			std::cout << "Playing animation " << anim.first << std::endl;
		}
	}
}

void setTimeToLastKeyframe(float &time, const string &current_animation_name)
{
	if (!name_to_animations[current_animation_name].empty())
		time = name_to_animations[current_animation_name].rbegin()->first;
	else
		time = 0.0f;
}

std::map<string, Animations> loadAnimationsFromDir(string dir_path)
{
	std::map<string, Animations> animations;
	for (const auto &entry : std::filesystem::directory_iterator(dir_path))
	{
		if (entry.path().extension() != ".anim")
			throw std::runtime_error("Invalid file type");

		auto path = entry.path();
		path = path.replace_extension("");
		Animations a = loadAnimations(path);

		if (!a.empty())
		{
			auto path = entry.path();
			string name = path.stem();
			animations[name] = a;
		}
	}

	return animations;
}

Animations loadAnimations(const string name)
{
	std::ifstream file(name + ".anim");

	if (!file.is_open())
	{
		std::cerr << "Animation " << name << " not found" << std::endl;
		return Animations();
	}

	std::stringstream buffer;

	buffer << file.rdbuf();

	Animations animation;
	std::vector<string> frames = split_set(buffer.str(), "~");

	for (string frame : frames)
	{
		float time;

		try
		{
			time = std::stof(frame.substr(0, frame.find_first_of("\n")));
			frame.erase(0, frame.find_first_of("\n") + 1);
		}
		catch (std::exception &e)
		{
			std::cerr << "Error loading animation time of: " << name << std::endl;
			std::cerr << e.what() << std::endl;
			return Animations();
		}

		try
		{
			std::vector<string> string_animations = split_set(frame, "\n");

			animation[time] = parseAnimations(string_animations);
		}
		catch (std::exception &e)
		{
			std::cerr << "Error loading animation transforms of: " << name << std::endl;
			std::cerr << e.what() << std::endl;
			return Animations();
		}
	}

	file.close();

	std::cout << "Animation " << name << " loaded" << std::endl;

	return animation;
}

std::vector<Animation> parseAnimations(std::vector<string> string_animations)
{
	std::vector<Animation> animations;

	for (string transform : string_animations)
	{
		Animation a;
		std::stringstream ss(transform);
		ss >> a;
		animations.push_back(a);
	}

	return animations;
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

void runAnimations(Bone *root, Animations &a, std::chrono::_V2::system_clock::time_point start)
{
	float t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0f;
	auto before = a.lower_bound(t);
	auto after = a.upper_bound(t);

	if (before != a.begin())
		before--;
	if (after == a.end())
		after--;
	t = (t - before->first) / (after->first - before->first);

	std::vector<Animation> animations;

	for (size_t i = 0; i < before->second.size(); i++)
	{
		Animation animation;

		if (std::isinf(t))
			animation = before->second[i];
		else
			animation = linear_interpolation(before->second[i], after->second[i], t);

		animations.push_back(animation);
	}

	root->applyAnimations(animations);
}
