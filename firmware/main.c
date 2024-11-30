/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <hal.h>
#include <test.h>
#include <serial-protocol.h>

#ifndef USART_BAUDRATE
#define USART_BAUDRATE 115200
#endif

extern uint8_t test_count;


static inline void
serial_send_byte(char c)
{
    while ((USART_REG->ISR & USART_ISR_TXE) != USART_ISR_TXE);
    USART_REG->TDR = c;
}


int
main(void)
{
    hal_clock_init();

    RCC->GPIO_RCC |= GPIO_RCC_EN(GPIO_RCC, BOARD_LED_BLK) | GPIO_RCC_EN(GPIO_RCC, TEST_LEDS_BLK);

    BOARD_LED_REG->MODER &= ~(GPIO_MODER_MASK(BOARD_LED_IDX));
    BOARD_LED_REG->MODER |= GPIO_MODER_OUTPUT(BOARD_LED_IDX);
    board_led_reset();

    TEST_LEDS_REG->MODER &= ~(GPIO_MODER_MASK(TEST_LED1_IDX) | GPIO_MODER_MASK(TEST_LED2_IDX) |
                              GPIO_MODER_MASK(TEST_LED3_IDX) | GPIO_MODER_MASK(TEST_LED4_IDX));
    TEST_LEDS_REG->MODER |= GPIO_MODER_OUTPUT(TEST_LED1_IDX) | GPIO_MODER_OUTPUT(TEST_LED2_IDX) |
                            GPIO_MODER_OUTPUT(TEST_LED3_IDX) | GPIO_MODER_OUTPUT(TEST_LED4_IDX);
    test_leds_reset(TEST_LEDS);

    RCC->GPIO_RCC  |= GPIO_RCC_EN(GPIO_RCC, USART_BLK);
    RCC->USART_RCC |= USART_RCC_EN(USART_RCC, USART_IDX);

    USART_GPIO_REG->OTYPER |= GPIO_OTYPER_OT(USART_TX_IDX) | GPIO_OTYPER_OT(USART_RX_IDX);
    USART_GPIO_REG->PUPDR &= ~(GPIO_PUPDR_PUPD_MASK(USART_TX_IDX) | GPIO_PUPDR_PUPD_MASK(USART_RX_IDX));
    USART_GPIO_REG->PUPDR |= GPIO_PUPDR_PUPD_UP(USART_TX_IDX) | GPIO_PUPDR_PUPD_UP(USART_RX_IDX);
    USART_GPIO_REG->MODER &= ~(GPIO_MODER_MASK(USART_TX_IDX) | GPIO_MODER_MASK(USART_RX_IDX));
    USART_GPIO_REG->MODER |= GPIO_MODER_ALTERNATE(USART_TX_IDX) | GPIO_MODER_ALTERNATE(USART_RX_IDX);
    USART_GPIO_REG->AFR[USART_TX_AFR_IDX] &= ~(GPIO_AFR_AFSEL_MASK(USART_TX_AFR_REG, USART_TX_IDX));
    USART_GPIO_REG->AFR[USART_TX_AFR_IDX] |= GPIO_AFR_AFSEL_USART(USART_TX_AFR_REG, USART_TX_IDX);
    USART_GPIO_REG->AFR[USART_RX_AFR_IDX] &= ~(GPIO_AFR_AFSEL_MASK(USART_RX_AFR_REG, USART_RX_IDX));
    USART_GPIO_REG->AFR[USART_RX_AFR_IDX] |= GPIO_AFR_AFSEL_USART(USART_RX_AFR_REG, USART_RX_IDX);

    USART_REG->BRR = (uint16_t) (SystemCoreClock / USART_BAUDRATE);
    USART_REG->CR3 = USART_CR3_ONEBIT;
    USART_REG->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    while (((USART_REG->ISR & USART_ISR_REACK) != USART_ISR_REACK) &&
           ((USART_REG->ISR & USART_ISR_TEACK) != USART_ISR_TEACK));

    if (test_init)
        test_init();

    while (true) {
        if ((USART_REG->ISR & USART_ISR_RXNE) == USART_ISR_RXNE) {
            uint8_t reg = USART_REG->RDR;
            uint8_t cmd = reg & 0xf0;
            uint8_t val = reg & 0x0f;

            switch ((serial_proto_cmd_t) (cmd >> 4)) {
            case PROTO_CMD_CONFIGURE_TEST:
                if (val == 0 || val > test_count) {
                    serial_send_byte(cmd | PROTO_REPLY_CONFIGURE_TEST_INVALID_ID);
                    break;
                }
                serial_send_byte(cmd | (test_configure(val) ?
                    PROTO_REPLY_CONFIGURE_TEST_OK : PROTO_REPLY_CONFIGURE_TEST_FAILED));
                break;

            case PROTO_CMD_GET_TEST_COUNT:
                serial_send_byte(cmd | (test_count & 0x0f));
                break;

            case PROTO_CMD_SYNC:
                switch ((serial_proto_cmd_sync_t) val) {
                case PROTO_CMD_SYNC_VAL_SYNC:
                    serial_send_byte(reg);
                    break;
                }
                break;
            }
            continue;
        }

        if (test_loop)
            test_loop();
    }

    return 0;
}
