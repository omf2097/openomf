#include <CUnit/CUnit.h>
#include <stdio.h>
#include <string.h>
#include "controller/net_controller.h"
#include "utils/iterator.h"

// Test data structure
static wtf test_data;

// Helper function to initialize wtf data for testing
static void wtf_init(wtf *data) {
    memset(data, 0, sizeof(wtf));
    data->id = 0;
    data->host = NULL;
    data->peer = NULL;
    data->lobby = NULL;
    data->last_hb = 0;
    data->outstanding_hb = 0;
    data->disconnected = 0;
    data->rttpos = 0;
    data->tick_offset = 0;
    data->rttfilled = 0;
    data->synchronized = false;
    data->local_proposal = 0;
    data->peer_proposal = 0;
    data->confirmed = false;
    data->last_tick = 0;
    data->last_sent_tick = 0;
    data->gs_bak = NULL;
    data->last_received_tick = 0;
    data->last_acked_tick = 0;
    data->last_har_state = -1;
    data->trace_file = NULL;
    data->last_traced_tick = 0;
    data->winner = -1;
    data->last_action = ACT_NONE;
    data->last_peer_action = ACT_NONE;
    data->last_peer_input_tick = 0;
    data->frame_advantage = 0;
    data->guesses = 0;
    data->peer_last_hash = 0;
    data->peer_last_hash_tick = 0;
    data->last_hash = 0;
    data->last_hash_tick = 0;
    data->last_rewind_tick = 0;
    list_create(&data->transcript);
}

// Helper function to initialize test data
static void setup_test_data() {
    wtf_init(&test_data);
}

// Helper function to cleanup test data
static void cleanup_test_data() {
    list_free(&test_data.transcript);
}

// Helper function to count events in transcript
static int count_transcript_events() {
    iterator it;
    list_iter_begin(&test_data.transcript, &it);
    int count = 0;
    tick_events *ev = NULL;
    while((ev = iter_next(&it)) != NULL) {
        count++;
    }
    return count;
}

// Helper function to find event at specific tick
static tick_events* find_event_at_tick(uint32_t tick) {
    iterator it;
    list_iter_begin(&test_data.transcript, &it);
    tick_events *ev = NULL;
    while((ev = iter_next(&it)) != NULL) {
        if(ev->tick == tick) {
            return ev;
        }
    }
    return NULL;
}

// Helper function to export binary data for testing (matches Erlang encode_inputs format)
static int export_transcript_binary(wtf *data, uint8_t *buffer, size_t buffer_size) {
    if (!data || !buffer) return -1;
    
    iterator it;
    list_iter_begin(&data->transcript, &it);
    tick_events *ev = NULL;
    size_t offset = 0;
    
    foreach(it, ev) {
        // Check buffer space: 4 bytes tick + events + terminators
        size_t needed = 4 + 2; // minimum: tick + 2 terminators
        
        // Count actual events for both players
        int p0_count = 0, p1_count = 0;
        for (int i = 0; i < MAX_EVENTS_PER_TICK && ev->events[0][i] != 0; i++) p0_count++;
        for (int i = 0; i < MAX_EVENTS_PER_TICK && ev->events[1][i] != 0; i++) p1_count++;
        needed += p0_count + p1_count;
        
        if (offset + needed > buffer_size) return -1; // Buffer too small
        
        // Write tick as 32-bit big-endian
        buffer[offset++] = (ev->tick >> 24) & 0xFF;
        buffer[offset++] = (ev->tick >> 16) & 0xFF;
        buffer[offset++] = (ev->tick >> 8) & 0xFF;
        buffer[offset++] = ev->tick & 0xFF;
        
        // Write challenger events (player 0) + terminator
        for (int i = 0; i < p0_count; i++) {
            buffer[offset++] = ev->events[0][i];
        }
        buffer[offset++] = 0; // terminator
        
        // Write challengee events (player 1) + terminator  
        for (int i = 0; i < p1_count; i++) {
            buffer[offset++] = ev->events[1][i];
        }
        buffer[offset++] = 0; // terminator
    }
    
    return offset; // Return number of bytes written
}

// Test basic event insertion
static void test_basic_event_insertion() {
    setup_test_data();

    // Insert a basic event
    insert_event(&test_data, 100, 48, 0);  // RIGHT action for player 0 at tick 100

    CU_ASSERT_EQUAL(count_transcript_events(), 1);

    tick_events *ev = find_event_at_tick(100);
    CU_ASSERT_PTR_NOT_NULL(ev);
    CU_ASSERT_EQUAL(ev->tick, 100);
    CU_ASSERT_EQUAL(ev->events[0][0], 48);
    CU_ASSERT_EQUAL(ev->events[0][1], 0);  // Should be terminated
    CU_ASSERT_EQUAL(ev->events[1][0], 0);  // Player 1 should have no events

    cleanup_test_data();
}

// Test zero-action event insertion (critical for sync)
static void test_zero_action_insertion() {
    setup_test_data();

    // Insert zero-action events like we see at tick 2 in the REC files
    insert_event(&test_data, 2, 0, 0);  // Player 0, no action
    insert_event(&test_data, 2, 0, 1);  // Player 1, no action

    CU_ASSERT_EQUAL(count_transcript_events(), 0);  // Both zero-actions filtered as duplicates

    // No events should exist since both zero-actions are filtered
    tick_events *ev = find_event_at_tick(2);
    CU_ASSERT_PTR_NULL(ev);

    cleanup_test_data();
}

// Test duplicate event filtering
static void test_duplicate_filtering() {
    setup_test_data();

    // Insert same action twice for same player - should be deduplicated
    insert_event(&test_data, 100, 48, 0);
    insert_event(&test_data, 100, 48, 0);  // Duplicate

    CU_ASSERT_EQUAL(count_transcript_events(), 1);

    tick_events *ev = find_event_at_tick(100);
    CU_ASSERT_PTR_NOT_NULL(ev);
    CU_ASSERT_EQUAL(ev->events[0][0], 48);
    CU_ASSERT_EQUAL(ev->events[0][1], 0);  // Should not have duplicate

    cleanup_test_data();
}

// Test multiple actions on same tick
static void test_multiple_actions_same_tick() {
    setup_test_data();

    // Insert different actions for same player on same tick
    insert_event(&test_data, 100, 48, 0);   // RIGHT
    insert_event(&test_data, 100, 49, 0);   // PUNCH|RIGHT

    CU_ASSERT_EQUAL(count_transcript_events(), 1);

    tick_events *ev = find_event_at_tick(100);
    CU_ASSERT_PTR_NOT_NULL(ev);
    CU_ASSERT_EQUAL(ev->events[0][0], 48);
    CU_ASSERT_EQUAL(ev->events[0][1], 49);
    CU_ASSERT_EQUAL(ev->events[0][2], 0);  // Terminated

    cleanup_test_data();
}

// Test events for both players on same tick
static void test_both_players_same_tick() {
    setup_test_data();

    // Insert events for both players on same tick
    insert_event(&test_data, 100, 48, 0);   // Player 0: RIGHT
    insert_event(&test_data, 100, 112, 1);  // Player 1: LEFT

    CU_ASSERT_EQUAL(count_transcript_events(), 1);

    tick_events *ev = find_event_at_tick(100);
    CU_ASSERT_PTR_NOT_NULL(ev);
    CU_ASSERT_EQUAL(ev->events[0][0], 48);   // Player 0
    CU_ASSERT_EQUAL(ev->events[0][1], 0);    // Terminated
    CU_ASSERT_EQUAL(ev->events[1][0], 112);  // Player 1
    CU_ASSERT_EQUAL(ev->events[1][1], 0);    // Terminated

    cleanup_test_data();
}

// Test chronological ordering
static void test_chronological_ordering() {
    setup_test_data();

    // Insert events out of order
    insert_event(&test_data, 200, 48, 0);
    insert_event(&test_data, 100, 112, 0);
    insert_event(&test_data, 150, 80, 0);

    CU_ASSERT_EQUAL(count_transcript_events(), 3);

    // Verify events are in chronological order
    iterator it;
    list_iter_begin(&test_data.transcript, &it);
    tick_events *ev1 = iter_next(&it);
    tick_events *ev2 = iter_next(&it);
    tick_events *ev3 = iter_next(&it);

    CU_ASSERT_EQUAL(ev1->tick, 100);
    CU_ASSERT_EQUAL(ev2->tick, 150);
    CU_ASSERT_EQUAL(ev3->tick, 200);

    cleanup_test_data();
}

// Test peer deduplication logic
static void test_peer_deduplication() {
    setup_test_data();
    test_data.id = 0;  // We are player 0

    // Insert event for peer (player 1)
    insert_event(&test_data, 100, 48, 1);
    CU_ASSERT_EQUAL(test_data.last_peer_action, 48);
    CU_ASSERT_EQUAL(test_data.last_peer_input_tick, 100);

    // Insert same action for peer at later tick - should be deduplicated
    insert_event(&test_data, 101, 48, 1);

    // Should only have the first event, second should be filtered
    CU_ASSERT_EQUAL(count_transcript_events(), 1);

    tick_events *ev100 = find_event_at_tick(100);
    tick_events *ev101 = find_event_at_tick(101);

    CU_ASSERT_PTR_NOT_NULL(ev100);
    CU_ASSERT_PTR_NULL(ev101);

    cleanup_test_data();
}

// Test the exact scenario from the REC file desync
static void test_rec_file_scenario() {
    setup_test_data();

    // Simulate the initial events from the REC files
    insert_event(&test_data, 2, 0, 0);    // Entry 0: Tick=2, Player=0, Action=0
    insert_event(&test_data, 2, 0, 1);    // Entry 1: Tick=2, Player=1, Action=0
    insert_event(&test_data, 36, 48, 1);  // Entry 2: Tick=36, Player=1, Action=48 (RIGHT)

    CU_ASSERT_EQUAL(count_transcript_events(), 1);  // Only Tick 36 (Tick 2 zero-actions filtered)

    // Verify tick 2 is filtered out (zero-actions are duplicates of initial state)
    tick_events *ev2 = find_event_at_tick(2);
    CU_ASSERT_PTR_NULL(ev2);

    // Verify tick 36 has player 1 with RIGHT action
    tick_events *ev36 = find_event_at_tick(36);
    CU_ASSERT_PTR_NOT_NULL(ev36);
    CU_ASSERT_EQUAL(ev36->events[1][0], 48);
    CU_ASSERT_EQUAL(ev36->events[0][0], 0);  // Player 0 should be empty

    cleanup_test_data();
}

// Test consecutive duplicate filtering within same tick
static void test_consecutive_duplicate_filtering() {
    setup_test_data();

    // Insert an action, then same action again on same tick
    insert_event(&test_data, 100, 48, 0);
    insert_event(&test_data, 100, 48, 0);  // Should be filtered by consecutive check
    insert_event(&test_data, 100, 49, 0);  // Different action, should be added

    tick_events *ev = find_event_at_tick(100);
    CU_ASSERT_PTR_NOT_NULL(ev);
    CU_ASSERT_EQUAL(ev->events[0][0], 48);
    CU_ASSERT_EQUAL(ev->events[0][1], 49);
    CU_ASSERT_EQUAL(ev->events[0][2], 0);  // Should be terminated, no duplicate 48

    cleanup_test_data();
}

// Test comprehensive input stream with realistic network scenarios and binary output validation
static void test_comprehensive_binary_output() {
    setup_test_data();
    
    // Simulate realistic network conditions matching Erlang test:
    
    // Initial zero-actions (should be filtered)
    insert_event(&test_data, 10, 0, 0);  // Player 0, initial zero
    insert_event(&test_data, 10, 0, 1);  // Player 1, initial zero
    
    // First real actions
    insert_event(&test_data, 20, 48, 0); // Player 0, RIGHT
    insert_event(&test_data, 20, 112, 1); // Player 1, LEFT
    
    // More actions
    insert_event(&test_data, 25, 1, 0);  // Player 0, PUNCH
    insert_event(&test_data, 30, 0, 0);  // Player 0, zero after PUNCH
    insert_event(&test_data, 30, 49, 1); // Player 1, PUNCH|RIGHT
    
    // Duplicate packets (network retransmission) - should be filtered
    insert_event(&test_data, 20, 48, 0); // Duplicate: Player 0, RIGHT 
    insert_event(&test_data, 20, 112, 1); // Duplicate: Player 1, LEFT
    insert_event(&test_data, 25, 1, 0);  // Duplicate: Player 0, PUNCH
    
    // Late packets (out of order)
    insert_event(&test_data, 35, 80, 0); // Player 0, DOWN (late arrival)
    insert_event(&test_data, 32, 2, 1);  // Player 1, KICK (very late)
    
    // Overlapping batch (missing ACK scenario)
    insert_event(&test_data, 30, 49, 1); // Duplicate from earlier
    insert_event(&test_data, 35, 80, 0); // Duplicate from late packet
    insert_event(&test_data, 40, 112, 0); // New: Player 0, LEFT
    insert_event(&test_data, 40, 3, 1);  // New: Player 1, PUNCH|KICK
    
    // Second part of overlapping batch
    insert_event(&test_data, 45, 48, 0); // Player 0, RIGHT
    insert_event(&test_data, 45, 0, 1);  // Player 1, zero
    insert_event(&test_data, 50, 16, 0); // Player 0, DOWN
    
    // Multiple actions same tick
    insert_event(&test_data, 60, 48, 0); // Player 0, RIGHT
    insert_event(&test_data, 60, 49, 0); // Player 0, PUNCH|RIGHT (same tick)
    insert_event(&test_data, 60, 50, 0); // Player 0, KICK|RIGHT (same tick)
    insert_event(&test_data, 60, 112, 1); // Player 1, LEFT
    insert_event(&test_data, 60, 113, 1); // Player 1, PUNCH|LEFT (same tick)
    
    // Validate event count matches Erlang implementation
    int total_events = count_transcript_events();
    CU_ASSERT_EQUAL(total_events, 9);
    
    // Export binary data and validate exact match with Erlang
    uint8_t buffer[2048];
    int binary_size = export_transcript_binary(&test_data, buffer, sizeof(buffer));
    
    // Expected binary output (same as Erlang comprehensive test)
    const char *expected_binary_str = "0,0,0,20,48,0,112,0,0,0,0,25,1,0,0,0,0,0,30,0,49,0,0,0,0,32,0,2,0,0,0,0,35,80,0,0,0,0,0,40,112,0,3,0,0,0,0,45,48,0,0,0,0,0,50,16,0,0,0,0,0,60,48,49,50,0,112,113,0";
    
    // Generate actual binary string for comparison
    char actual_binary_str[4096] = {0};
    int pos = 0;
    for (int i = 0; i < binary_size; i++) {
        if (i > 0) {
            pos += snprintf(actual_binary_str + pos, sizeof(actual_binary_str) - pos, ",");
        }
        pos += snprintf(actual_binary_str + pos, sizeof(actual_binary_str) - pos, "%d", buffer[i]);
    }
    
    // Validate exact binary compatibility with Erlang implementation
    CU_ASSERT_EQUAL(binary_size, 69);
    CU_ASSERT_STRING_EQUAL(actual_binary_str, expected_binary_str);
    
    cleanup_test_data();
}

// Test suite function
void net_controller_test_suite(CU_pSuite suite) {
    CU_add_test(suite, "Basic event insertion", test_basic_event_insertion);
    CU_add_test(suite, "Zero-action insertion", test_zero_action_insertion);
    CU_add_test(suite, "Duplicate filtering", test_duplicate_filtering);
    CU_add_test(suite, "Multiple actions same tick", test_multiple_actions_same_tick);
    CU_add_test(suite, "Both players same tick", test_both_players_same_tick);
    CU_add_test(suite, "Chronological ordering", test_chronological_ordering);
    CU_add_test(suite, "Peer deduplication", test_peer_deduplication);
    CU_add_test(suite, "REC file scenario", test_rec_file_scenario);
    CU_add_test(suite, "Consecutive duplicate filtering", test_consecutive_duplicate_filtering);
    CU_add_test(suite, "Comprehensive binary output", test_comprehensive_binary_output);
}