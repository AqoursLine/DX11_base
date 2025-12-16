#pragma once

#include "movieTexture.h"

struct VideoTexture {

  static bool createAPI();
  static void destroyAPI();

  struct InternalData;
  InternalData* internal_data = nullptr;

  bool create(const char* filename);
  void destroy();
  bool update(float dt);

  void pause();
  void resume();
  bool hasFinished();
  MovieTexture* getTexture();
  float getAspectRatio() const;
};

