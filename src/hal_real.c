void lvgl_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    // Set the display window
    set_display_window(area->x1, area->y1, area->x2, area->y2);

    // Send the pixel data
    send_pixels_to_display((uint8_t *)color_p, lv_area_get_size(area) * sizeof(lv_color_t));

    // Notify LVGL that flushing is done
    lv_disp_flush_ready(disp_drv);
}