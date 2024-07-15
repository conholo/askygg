#pragma once

int main(int argc, char** argv)
{
	askygg::Log::Init();
	auto* app = askygg::CreateApplication({ argc, argv });
	app->Run();
	delete app;
}