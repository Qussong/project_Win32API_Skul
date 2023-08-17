#pragma once
#include "vanScene.h"

namespace van
{
	class Stage1Monster2Scene : public Scene
	{
	public:
		Stage1Monster2Scene();
		virtual ~Stage1Monster2Scene();

		virtual void Init() override;
		virtual void Update() override;
		virtual void Render(HDC _hdc) override;
		virtual void SceneIN() override;
		virtual void SceneOut() override;
		virtual void CameraMove() override;
		virtual void MakeWorld() override;
		virtual void MakeFloor() override;
		virtual void MakeWall() override;
		virtual void MakeDoor() override;

	private:
	};
}

