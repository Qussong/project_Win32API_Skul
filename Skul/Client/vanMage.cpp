#include "vanMage.h"
#include "vanRigidBody.h"
#include "vanAnimator.h"
#include "vanResourceManager.h"
#include "vanTexture.h"
#include "vanCollider.h"
#include "vanTransform.h"
#include "vanWall.h"
#include "vanFireBall.h"
#include "vanRangeFire.h"
#include "vanObject.h"
#include "vanPhoenixLanding.h"
#include "vanPhoenixRandingReady.h"
#include "vanPlayerAttack.h"
#include "vanSkull.h"
#include "vanFinishMoveReady.h"
#include "vanWorldOnFireOrb.h"


#define MAX_HP					500.0f
#define FLY_POS					150.0f
#define WALK_SPEED				200.0f
#define FLY_SPEED				300.0f
#define LANDING_SPEED			400.0f
#define PHOENIX_READY_GAP_POS	100.0f
#define FLY_READY_GAP_POS		150.0f
//#define LANDING_TIMER			60.0f
#define LANDING_TIMER			5.0f
#define	MAX_ORB_CNT				3
#define DAMAGE_PERCENT			0.5f
namespace van
{
	Mage::Mage()
	{
		AddComponent<RigidBody>();
	}

	Mage::~Mage()
	{
		// nothing
	}

	void Mage::Init()
	{
		Boss::Init();

		MakeAnimation();

		SetBossState(BossState::Gen);
		SetBossDirection(BossDirection::Left);
		SetMaxHp(MAX_HP);
		SetHp(MAX_HP);

		Collider* col = GetComponent<Collider>();
		col->SetSize(math::Vector2(33.0f * 2, 65.0f * 2));

		Animator* at = GetComponent<Animator>();
		at->SetScale(math::Vector2(2.0f, 2.0f));
	}

	void Mage::Update()
	{
		Boss::Update();

		// 강제로 데미지 주기
		CmdDamage();
		// Boss의 Attack Direction이 달라지는지 확인
		ComparePosWithBossAndTarget();
		// LandingTimer 카운트
		LandingTimerCnt();
		
		mBossPastState = GetBossState();
		mBossPastDirection = GetBossDirection();

		if (GetBossState() == BossState::Dead)
		{
			Dead();
		}
		else
		{
			switch (GetBossState())
			{
			case BossState::Gen:
				Gen();
				break;
			case BossState::Idle:
				Idle();
				break;
			case BossState::Walk:
				Walk();
				break;
			case BossState::AttackReady:
				AttackReady();
				break;
			case BossState::Attack:
				Attack();
				break;
			case BossState::AttackEnd:
				AttackEnd();
				break;
			case BossState::Hit:
				Hit();
				break;
			case BossState::Dead:
				Dead();
				break;
			case BossState::TakeOff:
				TakeOff();
				break;
			default:
				__noop;
			}
		}

		// Dead 상태로 바껴야하는지 확인
		ChekDead();

		// Boss의 State(상태)가 달라지면 새로운 애니메이션 재생
		if (mBossPastState != GetBossState()
			|| mBossPastDirection != GetBossDirection())
		{
			mbPlayAnimation = true;
		}
	}

	void Mage::Render(HDC _hdc)
	{
		Boss::Render(_hdc);
	}

	void Mage::MakeAnimation()
	{
		math::Vector2 offset = math::Vector2(0.0f, -15.0f);

		Animator* at = GetComponent<Animator>();
		at->CreateAnimation(L"Intor_1", ResourceManager::Find<Texture>(L"Mage_Intor_1"), math::Vector2(0.0f, 0.0f), math::Vector2(33.0f, 81.0f), 40);
		at->CreateAnimation(L"Intor_2", ResourceManager::Find<Texture>(L"Mage_Intor_2"), math::Vector2(0.0f, 0.0f), math::Vector2(42.0f, 96.0f), 10, math::Vector2(0.0f, -8.0f));
		at->CreateAnimation(L"Idle_L", ResourceManager::Find<Texture>(L"Mage_Idle_L"), math::Vector2(0.0f, 0.0f), math::Vector2(41.0f, 71.0f), 6, math::Vector2(0.0f, 0.0f));
		at->CreateAnimation(L"Idle_R", ResourceManager::Find<Texture>(L"Mage_Idle_R"), math::Vector2(0.0f, 0.0f), math::Vector2(41.0f, 71.0f), 6, math::Vector2(0.0f, 0.0f));
		at->CreateAnimation(L"Walk_Front_L", ResourceManager::Find<Texture>(L"Mage_Walk_Front_L"), math::Vector2(0.0f, 0.0f), math::Vector2(57.0f, 55.0f), 6);
		at->CreateAnimation(L"Walk_Front_R", ResourceManager::Find<Texture>(L"Mage_Walk_Front_R"), math::Vector2(0.0f, 0.0f), math::Vector2(57.0f, 55.0f), 6);
		at->CreateAnimation(L"Walk_Back_L", ResourceManager::Find<Texture>(L"Mage_Walk_Back_L"), math::Vector2(0.0f, 0.0f), math::Vector2(60.0f, 60.0f), 6);
		at->CreateAnimation(L"Walk_Back_R", ResourceManager::Find<Texture>(L"Mage_Walk_Back_R"), math::Vector2(0.0f, 0.0f), math::Vector2(60.0f, 60.0f), 6);

		at->CreateAnimation(L"Attack_Ready_FireBall_L", ResourceManager::Find<Texture>(L"Mage_FireBall_Ready_L"), math::Vector2(0.0f, 0.0f), math::Vector2(55.0f, 96.0f), 13, offset);
		at->CreateAnimation(L"Attack_Ready_FireBall_R", ResourceManager::Find<Texture>(L"Mage_FireBall_Ready_R"), math::Vector2(0.0f, 0.0f), math::Vector2(55.0f, 96.0f), 13, offset);
		at->CreateAnimation(L"Attack_FireBall_L", ResourceManager::Find<Texture>(L"Mage_FireBall_L"), math::Vector2(0.0f, 0.0f), math::Vector2(52.0f, 69.0f), 8, math::Vector2(0.0f, 10.0f) + offset, 0.09F);
		at->CreateAnimation(L"Attack_FireBall_R", ResourceManager::Find<Texture>(L"Mage_FireBall_R"), math::Vector2(0.0f, 0.0f), math::Vector2(52.0f, 69.0f), 8, math::Vector2(0.0f, 10.0f) + offset, 0.09F);
		at->CreateAnimation(L"Attack_Ready_RangeFire_L", ResourceManager::Find<Texture>(L"Mage_RangeFire_Ready_L"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 15, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"Attack_Ready_RangeFire_R", ResourceManager::Find<Texture>(L"Mage_RangeFire_Ready_R"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 15, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"Attack_RangeFire_L", ResourceManager::Find<Texture>(L"Mage_RangeFire_L"), math::Vector2(0.0f, 0.0f), math::Vector2(62.0f, 109.0f), 3, math::Vector2(0.0f, 8.0f) + offset);
		at->CreateAnimation(L"Attack_RangeFire_R", ResourceManager::Find<Texture>(L"Mage_RangeFire_R"), math::Vector2(0.0f, 0.0f), math::Vector2(62.0f, 109.0f), 3, math::Vector2(0.0f, 8.0f) + offset);

		at->CreateAnimation(L"Attack_Ready_PhoenixLanding_L", ResourceManager::Find<Texture>(L"Mage_PhoenixRanding_Ready_L"), math::Vector2(0.0f, 0.0f), math::Vector2(58.0f, 58.0f), 3);
		at->CreateAnimation(L"Attack_Ready_PhoenixLanding_R", ResourceManager::Find<Texture>(L"Mage_PhoenixRanding_Ready_R"), math::Vector2(0.0f, 0.0f), math::Vector2(58.0f, 58.0f), 3);
		at->CreateAnimation(L"Attack_PhoenixLanding_L", ResourceManager::Find<Texture>(L"Mage_PhoenixRanding_Land_L"), math::Vector2(0.0f, 0.0f), math::Vector2(47.0f, 54.0f), 9, math::Vector2(0.0f, 7.0f));
		at->CreateAnimation(L"Attack_PhoenixLanding_R", ResourceManager::Find<Texture>(L"Mage_PhoenixRanding_Land_R"), math::Vector2(0.0f, 0.0f), math::Vector2(47.0f, 54.0f), 9, math::Vector2(0.0f, 7.0f));
		
		at->CreateAnimation(L"Die_L", ResourceManager::Find<Texture>(L"Mage_Die_L"), math::Vector2(0.0f, 0.0f), math::Vector2(70.0f, 41.0f), 5, math::Vector2(0.0f, 20.0f));
		at->CreateAnimation(L"Die_R", ResourceManager::Find<Texture>(L"Mage_Die_R"), math::Vector2(0.0f, 0.0f), math::Vector2(70.0f, 41.0f), 5, math::Vector2(0.0f, 20.0f));

		at->CreateAnimation(L"FinishMove_Ready_L", ResourceManager::Find<Texture>(L"Mage_FinishMove_Ready_L"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 9, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_Ready_R", ResourceManager::Find<Texture>(L"Mage_FinishMove_Ready_R"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 9, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_Ready_Re_L", ResourceManager::Find<Texture>(L"Mage_FinishMove_Ready_Re_L"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 3, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_Ready_Re_R", ResourceManager::Find<Texture>(L"Mage_FinishMove_Ready_Re_R"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 3, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_L", ResourceManager::Find<Texture>(L"Mage_FinishMove_L"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 3, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_R", ResourceManager::Find<Texture>(L"Mage_FinishMove_R"), math::Vector2(0.0f, 0.0f), math::Vector2(61.0f, 109.0f), 3, math::Vector2(0.0f, 15.0f) + offset);
		at->CreateAnimation(L"FinishMove_Re_L", ResourceManager::Find<Texture>(L"Mage_FinishMove_Re_L"), math::Vector2(0.0f, 0.0f), math::Vector2(62.0f, 109.0f), 3, math::Vector2(0.0f, 10.0f) + offset);
		at->CreateAnimation(L"FinishMove_Re_R", ResourceManager::Find<Texture>(L"Mage_FinishMove_Re_R"), math::Vector2(0.0f, 0.0f), math::Vector2(62.0f, 109.0f), 3, math::Vector2(0.0f, 10.0f) + offset);
	}

	void Mage::OnCollisionEnter(Collider* _other)
	{
		GameObject* obj = _other->GetOwner();
		// 벽에 충돌하면 Walk 방향을 바꿔준다.
		Wall* wall = dynamic_cast<Wall*>(obj);

		if (wall != nullptr)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				SetBossDirection(BossDirection::Right);
			}
			else if (GetBossDirection() == BossDirection::Right)
			{
				SetBossDirection(BossDirection::Left);
			}

			// 새로운 애니메이션 재생 ex) Walk
			mbPlayAnimation = true;
		}

		// Player 의 스킬
		Skull* playerSkill = dynamic_cast<Skull*>(obj);
		if (playerSkill != nullptr)
		{
			LoseHp(playerSkill->GetSkillDamage());
		}
	}

	void Mage::OnCollisionStay(Collider* _other)
	{
		GameObject* obj = _other->GetOwner();

		// Player 의 공격
		PlayerAttack* playerAtk = dynamic_cast<PlayerAttack*>(obj);
		if (playerAtk != nullptr)
		{
			// PlayerAttack 클래스의 충돌체 저장 정보를 가져온다
			std::set<GameObject*>* list = playerAtk->GetAttackList();

			if (list->find(this) == list->end()
				&& playerAtk->GetActiveFlag() == true)
			{
				list->insert(this);
				LoseHp(playerAtk->GetPlayerAttackDamage());
			}
		}
	}

	void Mage::OnCollisionExit(Collider* _other)
	{
		// nothing
	}

	void Mage::Gen()
	{
		Animator* at = GetComponent<Animator>();
		Transform* tr = GetComponent<Transform>();
		RigidBody* rb = GetComponent<RigidBody>();

		// 공중으로 올라간 뒤에 중력의 영향을 안 받도록 해준다.
		if (mbIntroFlag == true
			&& rb->GetGround() == false)
		{
			rb->SetGround(true);
		}

		// Mage 가 땅에 붙는순간 애니메이션 재생 시작
		// 첫번째 Intro 애니메이션
		if (rb->GetGround() == true
			&& mbPlayAnimation == true
			&& mbIntroFlag == false)
		{
			at->PlayAnimation(L"Intor_1", false);
			mbPlayAnimation = false;
			mbIntroFlag = true;
		}

		// 두번째 Intro 애니메이션
		if (mbPlayAnimation == true
			&& mbIntroFlag == true
			&& mbIntroEndFlag == false)
		{
			// 공중으로 올려준다.
			tr->SetPosition(tr->GetPosition() - math::Vector2(0.0f, FLY_POS));
			mbSky = true;
			at->PlayAnimation(L"Intor_2", false);
			mbPlayAnimation = false;
			mbIntroEndFlag = true;
		}

		// Intro 종료
		if (at->IsActiveAnimationComplete() == true
			&& mbIntroEndFlag == true)
		{
			//mInitPos_Y = tr->GetPosition().y;	// 높이 위치 저장
			SetBossState(BossState::Idle);
		}

		// 현재 애니메이션 재생 완료시 다음 영상 재생
		if (at->IsActiveAnimationComplete() == true)
		{
			mbPlayAnimation = true;
		}
	}

	void Mage::Idle()
	{
		Animator* at = GetComponent<Animator>();
		
		CmdSkill();	// 강제 커맨드 스킬

		if (mbAnimationReDirectionFlag == true)
		{
			SetBossDirection(mBossAttackDirection);
			mbAnimationReDirectionFlag = false;
		}

		if (mbPlayAnimation == true)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"Idle_L", true);
			}

			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"Idle_R", true);
			}

			mbPlayAnimation = false;
		}

		AddTimer(Time::GetDeltaTime());

		// Idle 상태가 1.5초 유지되면
		if (GetTimer() >= 1.5f)
		{
			// 설정 초기화
			SetTimer(0.0f);
			mbAnimationReDirectionFlag = true;

			// 다음 행동 설정
			/*
				Walk,			// 0
				Attack_Ready,	// 1
				TakeOff			// 2
			*/
			int nextCase = 0;
			if (mbTakeOff == true)
			{
				nextCase = 2;
				mbTakeOff = false;	// 플레그값 꺼준다.
			}
			else
			{
				if (mbCmd == true)
				{
					nextCase = 1;
				}
				else
				{
					srand((UINT)time(NULL));
					nextCase = (rand() % 2);
					//nextCase = 1;	// test
				}
			}

			switch (nextCase)
			{
			case 0:
				SetBossState(BossState::Walk);
				break;
			case 1:
				SetBossState(BossState::AttackReady);
				break;
			case 2:
				SetBossState(BossState::TakeOff);
				break;
			default:
				__noop;
			}
		}
	}

	void Mage::Walk()
	{
		Animator* at = GetComponent<Animator>();
		Transform* tr = GetComponent<Transform>();
		math::Vector2 pos = tr->GetPosition();
		// 좌우 이동
		if (GetBossDirection() == BossDirection::Left)
		{
			pos.x -= WALK_SPEED * Time::GetDeltaTime();
			tr->SetPosition(pos);

			if (mbPlayAnimation == true)
			{
				if (mBossAttackDirection == BossDirection::Left)
				{
					at->PlayAnimation(L"Walk_Front_L", true);
				}
				if (mBossAttackDirection == BossDirection::Right)
				{
					at->PlayAnimation(L"Walk_Back_L", true);
				}

				mbPlayAnimation = false;
			}
		}

		if (GetBossDirection() == BossDirection::Right)
		{
			pos.x += WALK_SPEED * Time::GetDeltaTime();
			tr->SetPosition(pos);

			if (mbPlayAnimation == true)
			{
				if (mBossAttackDirection == BossDirection::Left)
				{
					at->PlayAnimation(L"Walk_Back_R", true);
				}
				if (mBossAttackDirection == BossDirection::Right)
				{
					at->PlayAnimation(L"Walk_Front_R", true);
				}

				mbPlayAnimation = false;
			}
		}
		
		// Walk 상태 3초 유지후 Idle 상태로 전환
		AddTimer(Time::GetDeltaTime());
		if (GetTimer() >= 3.0f)
		{
			SetTimer(0.0f);
			SetBossState(BossState::Idle);
		}
	}

	void Mage::AttackReady()
	{
		if (mbChooseSkill == false)
		{
			if (mbCmd == true)
			{
				mAttackCase = (BossSkill)mCmdSkill;
			}
			else
			{
				/*
					FireBall,		// 0
					RangeFire,		// 1
					PhoenixLanding,	// 2
					FinishMove		// 3
					None			// 4
				*/
				srand((UINT)time(NULL));
				mAttackCase = (BossSkill)(rand() % 4);
				//mAttackCase = (BossSkill)(3);	// test
			}
			mbChooseSkill = true;
		}

		if (mbChooseSkill == true)
		{
			if (mAttackCase == BossSkill::FinishMove)
			{
				// FinishMove 발동조건 : 체력 50% 이하
				if (GetHp() / MAX_HP * 100.0f <= 50.0f)
				{
					AttackFinishMoveReady();
				}
				else
				{
					SetBossState(BossState::AttackEnd);
				}
			}
			else
			{
				// 필살기 외의 다른 스킬이 선택되었을 때
				switch (mAttackCase)
				{
				case BossSkill::FireBall:
					AttackFireBallReady();
					break;
				case BossSkill::RangeFire:
					AttackRangeFireReady();
					break;
				case BossSkill::PhoenixLanding:
					AttackPhoenixLandingReady();
					break;
				default:
					__noop;
				}
			}
		}
	}

	void Mage::Attack()
	{
		switch (mAttackCase)
		{
		case BossSkill::FireBall:
			AttackFireBall();
			break;
		case BossSkill::RangeFire:
			AttackRangeFire();
			break;
		case BossSkill::PhoenixLanding:
			AttackPhoenixLanding();
			break;
		case BossSkill::FinishMove:
			AttackFinishMove();
			break;
		default:
			__noop;
		}
	}

	void Mage::AttackEnd()
	{
		mbChooseSkill = false;				// 공격스킬 선택여부 초기화(false = 선택해야함)
		mbAnimationReDirectionFlag = true;	// 플레이어 방향에 맞게 다시 애니메이션 방향 조정
		SetBossState(BossState::Idle);		// Idle 상태로 전환
		mbCmd = false;
		mCmdSkill = 0;
	}

	void Mage::Hit()
	{

	}

	void Mage::Dead()
	{
		Animator* at = GetComponent<Animator>();
		RigidBody* rb = GetComponent<RigidBody>();

		if (mbPlayAnimation == true)
		{
			if (mbSky == true)
			{
				rb->SetGround(false);
			}

			if (GetBossDirection() == Boss::BossDirection::Left)
			{
				at->PlayAnimation(L"Die_L", false);
			}
			if(GetBossDirection() == Boss::BossDirection::Right)
			{
				at->PlayAnimation(L"Die_L", false);
			}
			mbPlayAnimation = false;
		}
	}

	void Mage::AttackFireBallReady()
	{
		Animator* at = GetComponent<Animator>();
		
		if (mbPlayAnimation == true)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"Attack_Ready_FireBall_L");
			}

			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"Attack_Ready_FireBall_R");
			}

			mbPlayAnimation = false;
		}

		if (at->IsActiveAnimationComplete() == true)
		{
			SetBossState(BossState::Attack);
		}
	}

	void Mage::AttackRangeFireReady()
	{
		Animator* at = GetComponent<Animator>();

		if (mbPlayAnimation == true)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"Attack_Ready_RangeFire_L");
			}

			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"Attack_Ready_RangeFire_R");
			}

			mbPlayAnimation = false;
		}

		if (at->IsActiveAnimationComplete() == true)
		{
			SetBossState(BossState::Attack);
		}
	}

	void Mage::AttackPhoenixLandingReady()
	{
		if (mbSky == false)
		{
			SetBossState(BossState::AttackEnd);
			return;
		}

		Transform* tr = GetComponent<Transform>();
		RigidBody* rb = GetComponent<RigidBody>();
		Animator* at = GetComponent<Animator>();
		math::Vector2 pos = tr->GetPosition();

		if (mbRecordPosY == false)
		{
			mInitPos = pos;
			mbRecordPosY = true;
		}

		if (mbFly == true)
		{
			// 위로 이동( -pos.y)
			pos.y -= 100.0f * Time::GetDeltaTime();
			tr->SetPosition(pos);
			float gap = mInitPos.y - pos.y;
			if (fabs(gap) >= 200.0f)	// 특정위치에 도달시
			{
				mbFly = false;			// 더 이상 안 올라가도 된다.
			}
		}

		// 올라가지 않음 && 내려가지 않음 == 낙하수행 전
		if(mbFly == false
			&& mbLand == false)
		{
			// Ready 애니메이션 실행
			if (mbPlayAnimation == true)
			{
				// 이펙트 실행
				mReadyEffect = Object::Instantiate<PhoenixRandingReady>(enums::eLayerType::Boss_Mage_Effect);
				mReadyEffect->SetOwner(this);

				if (GetBossDirection() == BossDirection::Left)
				{
					at->PlayAnimation(L"Attack_Ready_PhoenixLanding_L");
				}
				if (GetBossDirection() == BossDirection::Right)
				{
					at->PlayAnimation(L"Attack_Ready_PhoenixLanding_R");
				}

				mbPlayAnimation = false;
			}

			// 애니메이션 완료시 낙하수행
			if (at->IsActiveAnimationComplete() == true
				&& mReadyEffect->GetEffectFinishFlag() == true)
			{
				mbLand = true;				// 낙하시작
				rb->SetGround(false);		// 착지확인을 위해 중력을 적용받도록 한다.
				rb->SetVelocity(math::Vector2(0.0f, 300.0f));
			}
		}
		
		// 낙하시
		if (mbLand == true)
		{
			// 땅에 도달하면
			if (rb->GetGround() == true)
			{
				mbSky = false;	// 땅에 있음을 알려준다.
				// Attack으로 전환
				SetBossState(BossState::Attack);
			}
		}
	}

	void Mage::AttackFinishMoveReady()
	{
		Animator* at = GetComponent<Animator>();

		// Finish Move 준비 애니메이션 재생
		if (mbPlayAnimation == true
			&& mbFinishMoveReadyRe == false)
		{
			// FinishMove Charge Effect 객체 생성
			readyEffect = Object::Instantiate<FinishMoveReady>(enums::eLayerType::Boss_Mage_Effect);
			readyEffect->SetOwner(this);

			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"FinishMove_Ready_L", false);
			}
			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"FinishMove_Ready_R", false);
			}

			mbPlayAnimation = false;
		}

		// Finish Move 준비_Re 애니메이션 재생
		if (at->IsActiveAnimationComplete() == true
			&& mbFinishMoveReadyRe == false)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"FinishMove_Ready_Re_L", true);
			}

			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"FinishMove_Ready_Re_R", true);
			}

			mbFinishMoveReadyRe = true;
		}

		// Finish Move 준비시간 카운트 (3초)
		if (mbFinishMoveReadyRe == true
			&& mbFinishMoveCharge == false)
		{
			mFinishMoveChargeTime += Time::GetDeltaTime();
			// 3초 이상 준비했다면...
			if (mFinishMoveChargeTime >= 3.0f)
			{
				mFinishMoveChargeTime = 0.0f;
				mbFinishMoveCharge = true;	// Effect 에도 전달해준다.
				mbPlayAnimation = true;		// FinishMove Attack_1 애니메이션을 재생하기 위해서
			}
		}

		// 이펙트가 종료되면
		if (mbFinishMoveCharge == true
			&& mbFinishMoveEffectFinish == true)
		{
			// FinishMove Ready 자세에서 FinishMove Attack_1 자세로 변경하여 재생
			if (mbPlayAnimation == true)
			{
				if (GetBossDirection() == BossDirection::Left)
				{
					at->PlayAnimation(L"FinishMove_L", false);
				}
				if (GetBossDirection() == BossDirection::Right)
				{
					at->PlayAnimation(L"FinishMove_R", false);
				}

				mbPlayAnimation = false;
			}

			if(at->IsActiveAnimationComplete() == true)
			{
				mbFinishMoveCharge = false;
				mbFinishMoveReadyRe = false;
				mbFinishMoveEffectFinish = false;
				SetBossState(BossState::Attack);
			}
		}
	}

	void Mage::AttackFireBall()
	{
		Animator* at = GetComponent<Animator>();
		Transform* tr = GetComponent<Transform>();

		if (mbPlayAnimation == true)
		{
			if (mBossAttackDirection == BossDirection::Left)
			{
				at->PlayAnimation(L"Attack_FireBall_L", true);
			}

			if (mBossAttackDirection == BossDirection::Right)
			{
				at->PlayAnimation(L"Attack_FireBall_R", true);
			}

			mbPlayAnimation = false;
		}

		// FireBall 딜레이
		if (mbShoot == false)
		{
			AddTimer(Time::GetDeltaTime());

			if (GetTimer() >= 0.3f)
			{
				SetTimer(0.0f);
				mbShoot = true;
			}
		}

		// FireBall 생성
		if (mFireBallCnt < 6
			&& mbShoot == true)
		{
			FireBall* fireBall = Object::Instantiate<FireBall>(enums::eLayerType::Boss_Mage_Skill_FireBall);
			fireBall->SetOwner(this);
			//fireBall->GetComponent<Transform>()->SetPosition(tr->GetPosition());

			mListFireBall.push_back(fireBall);

			++mFireBallCnt;
			mbShoot = false;
		}

		// FireBall 종료조건
		if (mFireBallCnt == 6)
		{
			mFireBallCnt = 0;
			mbShoot = true;
			mListFireBall.clear();
			SetBossState(BossState::AttackEnd);
		}
	}

	void Mage::AttackRangeFire()
	{
		Animator* at = GetComponent<Animator>();
		Transform* tr = GetComponent<Transform>();

		if (mbPlayAnimation == true)
		{
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"Attack_RangeFire_L", true);
			}

			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"Attack_RangeFire_R", true);
			}

			mbPlayAnimation = false;
		}

		// RangeFire 딜레이
		if (mbShoot == false)
		{
			AddTimer(Time::GetDeltaTime());

			if (mbShoot == false
				&& GetTimer() >= 0.7f)
			{
				SetTimer(0.0f);
				mbShoot = true;
			}
		}

		// RangeFire 생성
		if (mRangeFire < 3
			&& mbShoot == true)
		{
			RangeFire* rangeFire = Object::Instantiate<RangeFire>(enums::eLayerType::Boss_Mage_Skill_RangeFire);
			rangeFire->SetOwner(this);
			rangeFire->GetComponent<Transform>()->SetPosition(tr->GetPosition());

			mListRangeFire.push_back(rangeFire);

			++mRangeFire;
			mbShoot = false;
		}

		// RangeFire 종료조건
		if (mRangeFire == 3)
		{
			mRangeFire = 0;
			mbShoot = true;
			mListRangeFire.clear();
			SetBossState(BossState::AttackEnd);
		}
	}

	void Mage::AttackPhoenixLanding()
	{
		Animator* at = GetComponent<Animator>();
		
		if (mbPlayAnimation == true
			&& PhoenixLandingAnimation == true)
		{
			mAttackEffect = Object::Instantiate<PhoenixLanding>(enums::eLayerType::Boss_Mage_Skill_PhoenixLanding);
			mAttackEffect->SetOwner(this);

			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"Attack_PhoenixLanding_L");
			}
			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"Attack_PhoenixLanding_R");
			}

			mbPlayAnimation = false;
			PhoenixLandingAnimation = false;
		}

		if (at->IsActiveAnimationComplete()
			&& mbPhoenixLandingEffect == true)
		{
			mbRecordPosY = false;
			mbFly = true;
			mbLand = false;
			mbLandingTimer = true;	// Landing 시간 카운트시작
			PhoenixLandingAnimation = true;
			mbPhoenixLandingEffect = false;

			// mbSky, mInitPosY 는 다시 하늘로 오르고 난 후 초기화한다.
			SetBossState(BossState::AttackEnd);
		}
	}

	void Mage::AttackFinishMove()
	{
		Animator* at = GetComponent<Animator>();

		if (mbPlayAnimation == true)
		{
			// FinishMove Attack_Re 자세 재생
			if (GetBossDirection() == BossDirection::Left)
			{
				at->PlayAnimation(L"FinishMove_Re_L", true);
			}
			if (GetBossDirection() == BossDirection::Right)
			{
				at->PlayAnimation(L"FinishMove_Re_R", true);
			}
			mbPlayAnimation = false;
		}

		// Orb 생성 ( Max = 3)
		if (mbOrbGenFlag == true)
		{
			WorldOnFireOrb* orb = Object::Instantiate<WorldOnFireOrb>(enums::eLayerType::Boss_Mage_Effect);
			orb->SetOwner(this);
			orb->GetComponent<Transform>()->SetPosition(math::Vector2(math::Vector2(-15.0f + mOrbCnt * 350.0f, Window_Y / 2 - 200.0f)));

			++mOrbCnt;
			mbOrbGenFlag = false;
		}

		// Orb 개수 제한
		if (mOrbCnt >= MAX_ORB_CNT)
		{
			mOrbCnt = 0;
			mbOrbGenFlag = true;
			SetBossState(BossState::AttackEnd);
		}

		// Orb 생성 Delay
		if (mbOrbGenFlag == false)
		{
			mOrbGenDelay += Time::GetDeltaTime();
			if (mOrbGenDelay >= 1.5f)
			{
				mOrbGenDelay = 0.0f;
				mbOrbGenFlag = true;
			}
		}
	}

	void Mage::ComparePosWithBossAndTarget()
	{
		mBossPastAttackDirection = mBossAttackDirection;

		GameObject* target = GetTarget();

		if (target != nullptr)
		{
			Transform* tr_target = target->GetComponent<Transform>();
			math::Vector2 pos_target = tr_target->GetPosition();
			math::Vector2 pos_boss = GetComponent<Transform>()->GetPosition();
			float gapX = pos_target.x - pos_boss.x;	// + : right, - : left

			if (gapX > 0.0f)
			{
				mBossAttackDirection = BossDirection::Right;
			}
			if (gapX < 0.0f)
			{
				mBossAttackDirection = BossDirection::Left;
			}
		}

		// Boss의 Attack Direction이 달라지면 새로운 애니메이션 재생 ex) Walk, Attack
		if (GetBossState() == BossState::Walk
			|| GetBossState() == BossState::Attack)
		{
			if (mBossPastAttackDirection != mBossAttackDirection)
			{
				mbPlayAnimation = true;
			}
		}

	}

	void Mage::LandingTimerCnt()
	{
		if (mbLandingTimer == true)
		{
			mLandingTimer += Time::GetDeltaTime();
			if (mLandingTimer >= LANDING_TIMER)
			{
				mLandingTimer = 0.0f;		// 타이머 시간 초기화
				mbLandingTimer = false;		// 타이머 측정여부 초기화 (초기값 : 측정안함)
				mbTakeOff = true;			// 땅 -> 하늘 위치 초기화 ON
			}
		}
	}

	void Mage::TakeOff()
	{
		RigidBody* rb = GetComponent<RigidBody>();
		Transform* tr = GetComponent<Transform>();
		math::Vector2 dest = mInitPos;			// 목적지(y좌표)
		math::Vector2 pos = tr->GetPosition();	// 현재위치

		if (rb->GetGround() == false)
		{
			rb->SetGround(true);
		}

		pos.y -= 100.0f * Time::GetDeltaTime();
		tr->SetPosition(pos);

		// 현재 위치가 목적지보다 높거나 같아지는 경우
		if (pos.y <= dest.y)
		{
			mbSky = true;					// 하늘에 떠 있는 상태로 설정 (PhoenixLanding 사용 가능)
			mInitPos = math::Vector2::Zero;	// PhoenixLanding 사용시 초기위치값 초기화
			SetBossState(BossState::Idle);
		}
	}

	void Mage::ChekDead()
	{
		if (GetHp() <= 0.0f
			&& mbDead == false)
		{
			mbDead = true;
			SetBossState(BossState::Dead);
		}
	}

	void Mage::CmdDamage()
	{
		if (Input::GetKey(eKeyCode::M)
			&& Input::GetKeyDown(eKeyCode::D))
		{
			LoseHp(MAX_HP * DAMAGE_PERCENT);
		}
	}

	void Mage::CmdSkill()
	{
		if (Input::GetKey(eKeyCode::Q)
			&& Input::GetKeyDown(eKeyCode::W))
		{
			mbCmd = true;
			mCmdSkill = 0;
		}

		if (Input::GetKey(eKeyCode::Q)
			&& Input::GetKeyDown(eKeyCode::E))
		{
			mbCmd = true;
			mCmdSkill = 1;
		}

		if (Input::GetKey(eKeyCode::Q)
			&& Input::GetKeyDown(eKeyCode::R))
		{
			mbCmd = true;
			mCmdSkill = 2;
		}

		if (Input::GetKey(eKeyCode::Q)
			&& Input::GetKeyDown(eKeyCode::Y))
		{
			mbCmd = true;
			mCmdSkill = 3;
		}
	}
}