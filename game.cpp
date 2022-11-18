#pragma clang diagnostic push
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

struct SpaceGem {
  olc::vf2d position;
  olc::vf2d velocity;
  int gemType;
  SpaceGem(float x, float y, float dx, float dy, int g) {
    position.x = x;
    position.y = y;
    velocity.x = dx;
    velocity.y = dy;
    gemType = g;
  }
};

struct Bullet {
  olc::vf2d position;
  olc::vf2d velocity;
  Bullet(float x, float y, float dx, float dy) {
    position.x = x;
    position.y = y;
    velocity.x = dx;
    velocity.y = dy;
  }
};

struct Shooter {
  olc::vf2d position;
  olc::vf2d velocity;
  float fireRate;
  int fireCount;
  float timer;
  Shooter(float x, float y, float dx, float dy, float fr, int fc) {
    position.x = x;
    position.y = y;
    velocity.x = dx;
    velocity.y = dy;
    fireRate = fr;
    fireCount = fc;
    timer = 1.0f / fr;
  }
};

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Insert Game Name Here"; }

 private:
  float timer = 0;
  int frames = 0;
  int fps;

  std::vector<Bullet> enemyBullets;
  std::vector<Bullet> friendlyBullets;
  std::vector<Shooter> shooters;
  float shooterSpawnTimer = 0;

  std::vector<SpaceGem> spaceGems;
  float spaceGemSpawnTimer = 0;

  olc::vf2d shipPosition{(float) WINDOW_WIDTH / 2, WINDOW_HEIGHT - 100};
  olc::vf2d shipVelocity{0, 0};
  olc::vf2d shipAcceleration;

  float acceleration = 3000;
  float deceleration = 7.5;
  float maxSpeed = 1000;

  bool shipAlive = true;
  float explosionTimer = 0;
  int explosionFrames = 25;
  int explosionFrameRate = 20;

  void addBullets(int count, const olc::vf2d& start) {
    const float TWO_PI = 2 * 3.14159265;
    float offset = TWO_PI * (float) rand() / RAND_MAX;
    for (auto i = 0; i < count; i++) {
      enemyBullets.emplace_back(Bullet(start.x, start.y,
                               100 * cos(TWO_PI * (float)i / count + offset),
                               100 * sin(TWO_PI * (float)i / count + offset)));
    }
  }

  olc::Sprite* backgroundSprite;
  olc::Decal* backgroundDecal;

  olc::Sprite* bulletSprite;
  olc::Decal* bulletDecal;

  olc::Sprite* missileSprite;
  olc::Decal* missileDecal;

  olc::Sprite* shooterSprite;
  olc::Decal* shooterDecal;

  olc::Sprite* shipSprite;
  olc::Decal* shipDecal;

  olc::Sprite* explosionSprite;
  olc::Decal* explosionDecal;

  olc::Sprite* spaceGemSprite;
  olc::Decal* spaceGemDecal;

 public:
  bool OnUserCreate() override {
    /*
      Load resources here
    */

    backgroundSprite = new olc::Sprite("background.png");
    backgroundDecal = new olc::Decal(backgroundSprite);

    bulletSprite = new olc::Sprite("bullet.png");
    bulletDecal = new olc::Decal(bulletSprite);

    missileSprite = new olc::Sprite("missile.png");
    missileDecal = new olc::Decal(missileSprite);

    shooterSprite = new olc::Sprite("shooter.png");
    shooterDecal = new olc::Decal(shooterSprite);

    shipSprite = new olc::Sprite("ship.png");
    shipDecal = new olc::Decal(shipSprite);

    explosionSprite = new olc::Sprite("explosion.png");
    explosionDecal = new olc::Decal(explosionSprite);

    spaceGemSprite = new olc::Sprite("spacegems.png");
    spaceGemDecal = new olc::Decal(spaceGemSprite);

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    timer += fElapsedTime;
    frames++;
    if (timer > 1.0) {
      fps = frames;
      frames = 0;
      timer -= 1;
      std::cout << shooters.size() << " shooters, " << enemyBullets.size()
                << " enemyBullets, " << spaceGems.size() << " spaceGems."
                << std::endl;
    }

    shooterSpawnTimer -= fElapsedTime;
    if (shooterSpawnTimer < 0) {
      shooterSpawnTimer += 2;
      shooters.emplace_back((float) rand() / RAND_MAX * WINDOW_WIDTH, -20,
                                 0, 50, 0.25, 10);
    }

    spaceGemSpawnTimer -= fElapsedTime;
    if (spaceGemSpawnTimer < 0) {
      spaceGemSpawnTimer += 0.5;
      spaceGems.emplace_back((float)rand() / RAND_MAX * WINDOW_WIDTH, -20,
                                   0, 100, rand() % 4);
    }

    inputs();
    processes(fElapsedTime);
    outputs();

    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    } else {
      return true;
    }
  }

  void inputs() {
    /*
      Game controls goes here
    */

    shipAcceleration.x = 0;
    shipAcceleration.y = 0;
    if (GetKey(olc::Key::LEFT).bHeld) shipAcceleration.x -= acceleration;
    if (GetKey(olc::Key::RIGHT).bHeld) shipAcceleration.x += acceleration;
    if (GetKey(olc::Key::UP).bHeld) shipAcceleration.y -= acceleration;
    if (GetKey(olc::Key::DOWN).bHeld) shipAcceleration.y += acceleration;

    if (GetKey(olc::Key::R).bPressed) {
      shipAlive = true;
      shipVelocity = olc::vf2d{0, 0};
      shipPosition = olc::vf2d{WINDOW_WIDTH / 2, WINDOW_HEIGHT - 100};
      explosionTimer = 0;
    }

    if (GetMouse(0).bPressed && shipAlive) {
      if (timer > 0.1) {
         std::cout << "Should fire" << std::endl;
         friendlyBullets.emplace_back(
                 shipPosition.x + missileSprite->width / 2,
//                 shipPosition.y + (float) shipSprite->height + (float) missileSprite->width / 2,
                 0, 0, 50);
      }
    }
  }

  void processes(float fElapsedTime) {
    /*
      Game logic goes here
    */

    if (shipAlive) {
      shipVelocity += fElapsedTime * shipAcceleration;

      shipVelocity *= 1 - fElapsedTime * deceleration;

      if (shipVelocity.x < -maxSpeed) shipVelocity.x = -maxSpeed;
      if (shipVelocity.y < -maxSpeed) shipVelocity.y = -maxSpeed;
      if (shipVelocity.x > maxSpeed) shipVelocity.x = maxSpeed;
      if (shipVelocity.y > maxSpeed) shipVelocity.y = maxSpeed;

      shipPosition += fElapsedTime * shipVelocity;

      if (shipPosition.x < 25) {
        shipPosition.x = 25;
        shipVelocity.x = 0;
      }
      if (shipPosition.y < 25) {
        shipPosition.y = 25;
        shipVelocity.y = 0;
      }
      if (shipPosition.x > ScreenWidth() - 25) {
        shipPosition.x = ScreenWidth() - 25;
        shipVelocity.x = 0;
      }
      if (shipPosition.y > ScreenHeight() - 25) {
        shipPosition.y = ScreenHeight() - 25;
        shipVelocity.y = 0;
      }
    } else {
      explosionTimer += fElapsedTime;
    }

    for (auto& bullet : enemyBullets) {
      bullet.position += fElapsedTime * bullet.velocity;
    }

    for (auto& bullet : friendlyBullets) {
      bullet.position += fElapsedTime * bullet.velocity;
    }

    for (auto& spaceGem : spaceGems) {
      spaceGem.position += fElapsedTime * spaceGem.velocity;
    }

    for (auto i = 0; i < enemyBullets.size();) {
      bool shipHit = pow(shipPosition.x - enemyBullets[i].position.x, 2) +
                     pow(shipPosition.y - enemyBullets[i].position.y, 2) <
                     pow(25, 2);

      if (shipHit) shipAlive = false;

      if (shipHit || enemyBullets[i].position.x < -20 ||
          enemyBullets[i].position.y < -20 ||
          enemyBullets[i].position.x > WINDOW_WIDTH + 20 ||
          enemyBullets[i].position.y > WINDOW_HEIGHT + 20) {
        enemyBullets.erase(enemyBullets.begin() + i);
      } else {
        i++;
      }
    }

    for (auto i = 0; i < spaceGems.size();) {
      bool shipHit = pow(shipPosition.x - spaceGems[i].position.x, 2) +
                         pow(shipPosition.y - spaceGems[i].position.y, 2) <
                     pow(30, 2);

      if (shipHit || spaceGems[i].position.x < -20 ||
          spaceGems[i].position.y < -20 ||
          spaceGems[i].position.x > WINDOW_WIDTH + 20 ||
          spaceGems[i].position.y > WINDOW_HEIGHT + 20) {
        spaceGems.erase(spaceGems.begin() + i);
      } else {
        i++;
      }
    }

    for (auto& shooter : shooters) {
      shooter.position += fElapsedTime * shooter.velocity;
      shooter.timer -= fElapsedTime;
      if (shooter.timer < 0) {
        shooter.timer += 1.0f / shooter.fireRate;
        addBullets(shooter.fireCount, shooter.position);
      }
    }

    for (auto i = 0; i < shooters.size();) {
      if (shooters[i].position.x < -20 || shooters[i].position.y < -20 ||
          shooters[i].position.x > WINDOW_WIDTH + 20 ||
          shooters[i].position.y > WINDOW_HEIGHT + 20) {
        shooters.erase(shooters.begin() + i);
      } else {
        i++;
      }
    }
  }

  void outputs() {
    SetPixelMode(olc::Pixel::NORMAL);

    /*
      Game graphics drawn here
    */

    DrawDecal(olc::vf2d(0, 0), backgroundDecal);

    float scale = 0.0333;
    auto centrePoint = scale * olc::vf2d{(float)bulletSprite->width / 2,
                                         (float)bulletSprite->height / 2};

    for (const auto& bullet : enemyBullets) {
      DrawDecal(bullet.position - centrePoint, bulletDecal,
                olc::vf2d{scale, scale});
    }

    centrePoint = scale * olc::vf2d{(float)missileSprite->width / 2,
                                    (float)missileSprite->height / 2};

    for (const auto& bullet : friendlyBullets) {
      DrawDecal(bullet.position - centrePoint, missileDecal,
                olc::vf2d{scale, scale});
    }

    scale = 0.0333;
    centrePoint = scale * olc::vf2d{(float)shooterSprite->width / 2,
                                    (float)shooterSprite->height / 2};

    for (const auto& shooter : shooters) {
      DrawDecal(shooter.position - centrePoint, shooterDecal,
                olc::vf2d{scale, scale});
    }

    scale = 1;
    float spaceGemSize = (float)spaceGemSprite->width / 4;
    centrePoint =
        scale * olc::vf2d{spaceGemSize / 2, (float)spaceGemSprite->height / 2};

    for (const auto& spaceGem : spaceGems) {
      DrawPartialDecal(spaceGem.position - centrePoint, spaceGemDecal,
                       olc::vf2d{spaceGem.gemType * spaceGemSize, 0},
                       olc::vf2d{spaceGemSize * scale,
                                 spaceGemSprite->height * scale});
    }

    if (shipAlive) {
      scale = 1;
      centrePoint = scale * olc::vf2d{(float)shipSprite->width / 2,
                                      (float)shipSprite->height / 2};

      DrawDecal(shipPosition - centrePoint, shipDecal, olc::vf2d{scale, scale});
    } else if (explosionTimer < explosionFrames / explosionFrameRate) {
      float explosionSize = (float)explosionSprite->width / explosionFrames;
      int explosionFrame = explosionTimer * explosionFrameRate;
      centrePoint = olc::vf2d{explosionSize / 2, explosionSize / 2};

      DrawPartialDecal(shipPosition - centrePoint,
                       olc::vf2d{explosionSize, explosionSize}, explosionDecal,
                       olc::vf2d{explosionFrame * explosionSize, 0},
                       olc::vf2d{explosionSize, explosionSize});
    }

    if (fps > 0) {
      auto fpsPosition = olc::vi2d(WINDOW_WIDTH - 70, WINDOW_HEIGHT - 70);
      DrawStringDecal(fpsPosition, "FPS " + std::to_string(fps));
    }
  }

  bool OnUserDestroy() override {
    std::cout << "Closing game" << std::endl;
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1)) game.Start();
  return 0;
}

#pragma clang diagnostic pop