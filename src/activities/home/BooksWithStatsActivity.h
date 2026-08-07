#pragma once

#include <vector>

#include "RecentBooksStore.h"
#include "activities/Activity.h"
#include "activities/reader/BookReadingStats.h"
#include "activities/reader/GlobalReadingStats.h"
#include "util/ButtonNavigator.h"

class BooksWithStatsActivity final : public Activity {
 public:
  static size_t countRecentBooksWithStats();

  BooksWithStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  ButtonNavigator buttonNavigator;
  std::vector<RecentBook> booksWithStats;
  BookReadingStats currentBookStats;
  size_t selectorIndex = 0;
  GlobalReadingStats globalStats;
  GlobalReadingStats allDevicesStats;
  bool showAllDevicesStats = false;

  void loadBooksWithStats();
  void loadSelectedBookStats();
};
