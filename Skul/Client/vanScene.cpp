#include "vanScene.h"

namespace van
{
	Scene::Scene()
		: target(nullptr)
	{
		mLayers.resize((int)enums::eLayerType::End);
	}

	Scene::~Scene()
	{

	}

	void Scene::Init()
	{
	}

	void Scene::Update()
	{
		/*
			mLayers ���� Layer ��ü���� ������ �����ؼ� �������� ������ 
			�����ڷ� ó���Ѵ�. (�޸� ��뷮 ��)
		*/
		for (Layer& layer : mLayers)
			layer.Update();

	}

	void Scene::Render(HDC _hdc)
	{
		for (Layer& layer : mLayers)
			layer.Render(_hdc);
		Time::Render(_hdc);
		Text::PrintwString(_hdc, 10, 50, L"Title - z");
		Text::PrintwString(_hdc, 10, 70, L"Home - x");
		Text::PrintwString(_hdc, 10, 90, L"Stage - c");
		Text::PrintwString(_hdc, 10, 110, L"Ending - v");
	}

	void Scene::AddGameObject(enums::eLayerType type, GameObject* gameObj)
	{
		mLayers[(int)type].AddGameObject(gameObj);	// type�� �ش��ϴ� ���̾ GameObject ��ü ����
	}
}