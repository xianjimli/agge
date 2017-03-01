#include <agge.text/layout.h>

#include "helpers.h"
#include "mocks.h"

#include <iterator>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			font::metrics c_fm1 = { 10.0f, 2.0f, 2.0f };
			font::metrics c_fm2 = { 14.0f, 3.0f, 1.0f };
		}

		begin_test_suite( LayoutTests )
			test( EmptyLayoutHasEmptyBox )
			{
				mocks::font::char_to_index indices[] = { { L'A', 0 }, };
				mocks::font::glyph glyphs[] = { { { 0, 0 } }, };
				font::ptr f(new mocks::font(c_fm1, indices, glyphs));

				// INIT / ACT
				layout l(L"", f);

				// ACT
				box_r box = l.get_box();

				// ASSERT
				assert_equal(0.0f, box.w);
				assert_equal(0.0f, box.h);
			}

		
			test( SingleLineUnboundLayoutBoxEqualsSumOfAdvances )
			{
				// INIT
				mocks::font::char_to_index indices1[] = { { L'A', 1 }, { L'B', 0 }, { L'Q', 0 }, };
				mocks::font::glyph glyphs[] = {
					{ { 13, 0 } },
					{ { 11, 0 } },
				};
				font::ptr f1(new mocks::font(c_fm1, indices1, glyphs));

				// INIT / ACT
				layout l1(L"A", f1);
				layout l2(L"AAB", f1);
				layout l3(L"BQA", f1);

				// ACT
				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(11.0f, box1.w);
				assert_equal(35.0f, box2.w);
				assert_equal(37.0f, box3.w);

				// INIT
				mocks::font::char_to_index indices2[] = { { L'A', 0 }, { L'B', 0 }, { L'Q', 0 }, };
				font::ptr f2(new mocks::font(c_fm1, indices2, glyphs));

				// INIT / ACT
				layout l4(L"A", f2);
				layout l5(L"ABQABQABQ", f2);

				// ACT
				box_r box4 = l4.get_box();
				box_r box5 = l5.get_box();

				// ASSERT
				assert_equal(13.0f, box4.w);
				assert_equal(117.0f, box5.w);
			}


			test( SingleLineUnboundLayoutProducesSingleGlyphRuns )
			{
				// INIT
				mocks::font::char_to_index indices1[] = { { L'A', 1 }, { L'B', 0 }, { L'Q', 0 }, };
				mocks::font::char_to_index indices2[] = { { L'A', 0 }, { L'B', 1 }, { L'Q', 2 }, { L' ', 3 } };
				mocks::font::glyph glyphs[] = {
					{ { 13, 0 } },
					{ { 11, 0 } },
					{ { 12.7, 0 } },
					{ { 10.1, 0 } },
				};
				font::ptr f1(new mocks::font(c_fm1, indices1, glyphs));
				font::ptr f2(new mocks::font(c_fm1, indices2, glyphs));
				layout::const_iterator gr;

				// INIT / ACT
				layout l1(L"A", f1);
				layout l2(L"AAB", f1);
				layout l3(L"BQA", f1);
				layout l4(L"A", f2);
				layout l5(L"ABQ AQA", f2);

				// ASSERT
				layout::positioned_glyph reference1[] = { { 0.0f, 0.0f, 1 } };
				layout::positioned_glyph reference2[] = { { 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, };
				layout::positioned_glyph reference3[] = { { 0.0f, 0.0f, 0 }, { 13.0f, 0.0f, 0 }, { 13.0f, 0.0f, 1 }, };
				layout::positioned_glyph reference4[] = { { 0.0f, 0.0f, 0 }, };
				layout::positioned_glyph reference5[] = {
					{ 0.0f, 0.0f, 0 },
					{ 13.0f, 0.0f, 1 },
					{ 11.0f, 0.0f, 2 },
					{ 12.7f, 0.0f, 3 },
					{ 10.1f, 0.0f, 0 },
					{ 13.0f, 0.0f, 2 },
					{ 12.7f, 0.0f, 0 },
				};

				gr = l1.begin();
				assert_equal(1, distance(gr, l1.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference1, mkvector(gr->begin, gr->end));

				gr = l2.begin();
				assert_equal(1, distance(gr, l2.end()));
				assert_equal(f1, gr->glyph_run_font);
				assert_equal(reference2, mkvector(gr->begin, gr->end));

				gr = l3.begin();
				assert_equal(1, distance(gr, l3.end()));
				assert_equal(reference3, mkvector(gr->begin, gr->end));

				gr = l4.begin();
				assert_equal(1, distance(gr, l4.end()));
				assert_equal(f2, gr->glyph_run_font);
				assert_equal(reference4, mkvector(gr->begin, gr->end));

				gr = l5.begin();
				assert_equal(1, distance(gr, l5.end()));
				assert_equal(reference5, mkvector(gr->begin, gr->end));
			}


			test( MultiLineUnboundLayoutBoxEqualsMaxOfSumOfAdvancesInEachRow )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				font::ptr f(new mocks::font(c_fm1, indices, glyphs));

				// ACT
				layout l1(L"ABC CBA AB\nABB BBC\n", f);
				layout l2(L"AC CB\nA AB\nABB BBC\n", f);
				layout l3(L"AC CB\nA AB\nABB BBC", f); // Last row will be checked even if no newline is encountered.
				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(120.2f, box1.w);
				assert_equal(87.1f, box2.w);
				assert_equal(87.1f, box3.w);
			}


			test( MultiLineUnboundLayoutProducesGlyphRunsForEachLine )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				font::ptr f1(new mocks::font(c_fm1, indices, glyphs));
				font::ptr f2(new mocks::font(c_fm2, indices, glyphs));
				layout::const_iterator gr;

				// INIT / ACT
				layout l1(L"ABC CBA AB\nABB BBC\n", f1);
				layout l2(L"AC CB\nA AB\nABB BBC", f2);

				// ASSERT
				layout::positioned_glyph reference11[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 }, { 17.0f, 0.0f, 0 }, { 7.1f, 0.0f, 3 },
						{ 17.0f, 0.0f, 2 }, { 13.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, { 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference12[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 0 }, { 7.1f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 }, 
				};
				layout::positioned_glyph reference21[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 3 }, { 17.0f, 0.0f, 0 }, { 7.1f, 0.0f, 3 }, { 17.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference22[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 0 }, { 7.1f, 0.0f, 1 }, { 11.0f, 0.0f, 2 },
				};
				layout::positioned_glyph reference23[] = {
					{ 0.0f, 0.0f, 1 }, { 11.0f, 0.0f, 2 }, { 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 0 }, { 7.1f, 0.0f, 2 },
						{ 13.0f, 0.0f, 2 }, { 13.0f, 0.0f, 3 },
				};

				gr = l1.begin();
				assert_equal(2, distance(gr, l1.end()));
				assert_equal(mkpoint(0.0f, 10.0f), gr->reference);
				assert_equal(reference11, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 24.0f), gr->reference);
				assert_equal(reference12, mkvector(gr->begin, gr->end));

				gr = l2.begin();
				assert_equal(3, distance(gr, l2.end()));
				assert_equal(mkpoint(0.0f, 14.0f), gr->reference);
				assert_equal(reference21, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 32.0f), gr->reference);
				assert_equal(reference22, mkvector(gr->begin, gr->end));
				++gr;
				assert_equal(mkpoint(0.0f, 50.0f), gr->reference);
				assert_equal(reference23, mkvector(gr->begin, gr->end));
			}
		

			ignore( LongSingleLineIsBrokenOnWordBounds )
			{
				// INIT
				mocks::font::char_to_index indices[] = {
					{ L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'\'', 3 }, { L'C', 4 }, { L'.', 5 },
				};
				mocks::font::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 4.3, 0 } },
					{ { 13, 0 } },
					{ { 4, 0 } },
				};
				font::ptr f(new mocks::font(c_fm1, indices, glyphs));
				layout l1(L"AAAA BBBB CC BBBB AAAA", f);
				layout l2(L"CCC'C BBB AA AA AAAABBB CCCC AAAA ABABABABAB.", f);

				// ACT
				//l1.limit_width(11.1f);
				//l2.limit_width(11.1f);

				// ASSERT
				assert_equal(2, distance(l1.begin(), l1.end()));
			}

		end_test_suite
	}
}
