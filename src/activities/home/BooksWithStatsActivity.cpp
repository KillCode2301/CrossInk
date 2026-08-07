#include "BooksWithStatsActivity.h"

#include <I18n.h>

#include "BookActions.h"
#include "MappedInputManager.h"
#include "RecentBooksStore.h"
#include "activities/reader/BookStatsView.h"
#include "components/CompactHeader.h"
#include "components/UITheme.h"
#include "fontIds.h"

size_t BooksWithStatsActivity::countRecentBooksWithStats() {
  size_t count = 0;
  for (const auto& book : RECENT_BOOKS.getBooks()) {
    if (RecentBooksStore::isMissing(book)) {
      continue;
    }
    if (BookActions::bookHasReadingStats(book.path)) {
      count++;
    }
  }
  return count;
}

BooksWithStatsActivity::BooksWithStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("BooksWithStats", renderer, mappedInput) {}

void BooksWithStatsActivity::loadBooksWithStats() {
  booksWithStats.clear();
  const auto& books = RECENT_BOOKS.getBooks();
  booksWithStats.reserve(books.size());

  for (const auto& book : books) {
    if (RecentBooksStore::isMissing(book)) {
      continue;
    }
    if (!BookActions::bookHasReadingStats(book.path)) {
      continue;
    }
    booksWithStats.push_back(book);
  }

  if (booksWithStats.empty()) {
    selectorIndex = 0;
    return;
  }

  if (selectorIndex >= booksWithStats.size()) {
    selectorIndex = 0;
  }
}

void BooksWithStatsActivity::loadSelectedBookStats() {
  currentBookStats = {};
  if (selectorIndex >= booksWithStats.size()) {
    return;
  }

  const std::string cachePath = BookActions::bookStatsCachePath(booksWithStats[selectorIndex].path);
  if (cachePath.empty()) {
    return;
  }

  currentBookStats = BookReadingStats::load(cachePath);
}

void BooksWithStatsActivity::onEnter() {
  Activity::onEnter();

  if (RECENT_BOOKS.pruneMissing()) {
    RECENT_BOOKS.saveToFile();
  }

  globalStats = GlobalReadingStats::load();
  showAllDevicesStats = GlobalReadingStats::hasSyncedStats();
  allDevicesStats = showAllDevicesStats ? GlobalReadingStats::loadAggregated(globalStats) : globalStats;

  selectorIndex = 0;
  loadBooksWithStats();
  loadSelectedBookStats();
  requestUpdate();
}

void BooksWithStatsActivity::onExit() {
  Activity::onExit();
  booksWithStats.clear();
  currentBookStats = {};
}

void BooksWithStatsActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }

  const int listSize = static_cast<int>(booksWithStats.size());
  if (listSize <= 1) {
    return;
  }

  buttonNavigator.onNextRelease([this, listSize] {
    const int nextIndex = ButtonNavigator::nextIndex(static_cast<int>(selectorIndex), listSize);
    if (nextIndex == static_cast<int>(selectorIndex)) {
      return;
    }
    selectorIndex = static_cast<size_t>(nextIndex);
    loadSelectedBookStats();
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, listSize] {
    const int previousIndex = ButtonNavigator::previousIndex(static_cast<int>(selectorIndex), listSize);
    if (previousIndex == static_cast<int>(selectorIndex)) {
      return;
    }
    selectorIndex = static_cast<size_t>(previousIndex);
    loadSelectedBookStats();
    requestUpdate();
  });
}

void BooksWithStatsActivity::render(RenderLock&&) {
  if (booksWithStats.empty() || selectorIndex >= booksWithStats.size()) {
    renderer.clearScreen();
    const auto& metrics = UITheme::getInstance().getMetrics();
    CompactHeader::drawTitle(renderer, tr(STR_READING_STATS));
    renderer.drawText(UI_10_FONT_ID, metrics.contentSidePadding, CompactHeader::contentTop(metrics) + 20,
                      tr(STR_NO_ENTRIES));
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
    renderer.displayBuffer();
    return;
  }

  const RecentBook& book = booksWithStats[selectorIndex];
  renderBooksWithStatsHomePage(renderer, &mappedInput, book.title, currentBookStats, -1.0f, globalStats,
                               showAllDevicesStats ? &allDevicesStats : nullptr, booksWithStats.size() > 1);
  renderer.displayBuffer();
}
