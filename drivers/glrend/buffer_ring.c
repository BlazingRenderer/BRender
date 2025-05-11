#include "drv.h"
#include "brassert.h"

void BufferRingGLInit(br_buffer_ring_gl *self, const char *tag, size_t offset_alignment, size_t num_draws,
                      GLuint buffer_index, size_t elem_size)
{
    size_t aligned_size = ((sizeof(shader_data_model) + offset_alignment - 1) / offset_alignment) * offset_alignment;
    size_t buffer_size  = aligned_size * num_draws;

    assert(aligned_size <= elem_size);

    glGenBuffers(BR_ASIZE(self->buffers), self->buffers);
    for(int i = 0; i < BR_ASIZE(self->buffers); ++i) {
        glBindBuffer(GL_UNIFORM_BUFFER, self->buffers[i]);
        glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)buffer_size, NULL, GL_DYNAMIC_DRAW);
        DeviceGLObjectLabelF(GL_BUFFER, self->buffers[i], BR_GLREND_DEBUG_INTERNAL_PREFIX "ring:%s:%d", tag, i);

        self->fences[i] = NULL;
    }

    self->frame_index       = BR_GLREND_MODEL_RB_FRAMES;
    self->offset            = 0;
    self->buffer_size       = buffer_size;
    self->aligned_elem_size = (GLsizeiptr)aligned_size;
    self->buffer_index      = buffer_index;
}

void BufferRingGLFini(br_buffer_ring_gl *self)
{
    glDeleteBuffers(BR_ASIZE(self->buffers), self->buffers);

    for(size_t i = 0; i < BR_ASIZE(self->fences); ++i) {
        if(self->fences[i] != NULL) {
            glDeleteSync(self->fences[i]);
        }
    }
}

void BufferRingGLBegin(br_buffer_ring_gl *self)
{
    ++self->frame_index;
    if(self->frame_index >= BR_GLREND_MODEL_RB_FRAMES)
        self->frame_index = 0;

    self->offset = 0;

    if(self->fences[self->frame_index] != NULL) {
        GLenum result;
        do {
            result = glClientWaitSync(self->fences[self->frame_index], 0, UINT64_MAX);
        } while(result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED);

        glDeleteSync(self->fences[self->frame_index]);
        self->fences[self->frame_index] = NULL;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, self->buffers[self->frame_index]);
}

void BufferRingGLEnd(br_buffer_ring_gl *self)
{
    self->fences[self->frame_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

br_boolean BufferRingGLPush(br_buffer_ring_gl *self, const void *data, GLsizeiptr size)
{
    GLuint ubo = self->buffers[self->frame_index];

#if DEBUG
    {
        GLint actual_ubo;
        glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &actual_ubo);
        ASSERT(actual_ubo == ubo);
    }
#endif

    if(self->offset >= self->buffer_size)
        return BR_FALSE;

    glBufferSubData(GL_UNIFORM_BUFFER, self->offset, size, data);
    glBindBufferRange(GL_UNIFORM_BUFFER, self->buffer_index, ubo, self->offset, size);

    self->offset += self->aligned_elem_size;
    return BR_TRUE;
}
